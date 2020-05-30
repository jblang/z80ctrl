/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file bus.c Low-level control of the Z80 bus
 */

#include "bus.h"
#include "iox.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdio.h>

/**
 * Run the clock for a specified number of cycles
 */
void clk_cycle(uint8_t cycles)
{
    uint8_t i;
    for (i = 0; i < cycles; i++) {
        CLK_HI;
        CLK_LO;
    }
}

uint8_t clkdiv = 2;

/**
 * Run the Z80's clock
 */
void clk_run(void)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = (clkdiv - 1);
    OCR2B = (clkdiv - 1) >> 1;
}

/**
 * Stop the Z80's clock
 */
void clk_stop()
{
    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = 0;
    OCR2B = 0;
}

uint8_t bus_mode = BUS_SLAVE;

/**
 *  Request control of the bus from the Z80
 */
uint8_t bus_master(void)
{
    uint8_t i = 255;

    BUSRQ_LO;           // request bus
    // Clock the Z80 until it releases control of the bus
    while (GET_BUSACK)  {
        CLK_TOGGLE;
        if (i-- == 0) {
            printf_P(PSTR("bus master request timed out\n"));
            BUSRQ_HI;
            bus_mode = BUS_SLAVE;
            return 0;
        }
    }
    MREQ_HI;
    IORQ_HI;
    RD_HI;
    WR_HI;
    MREQ_OUTPUT;
    IORQ_OUTPUT;
    RD_OUTPUT;
    WR_OUTPUT;
    ADDR_OUTPUT;
    DATA_INPUT;
    bus_mode = BUS_MASTER;
    return 1;
}

/**
 *  Return control of the bus to the Z80
 */
void bus_slave(void)
{
    uint8_t i = 255;

    MREQ_INPUT;
    IORQ_INPUT;
    RD_INPUT;
    WR_INPUT;
    ADDR_INPUT;
    DATA_INPUT;
    SET_DATA(0); // Disable pullups on data and address lines
    SET_ADDRLO(0);
    BUSRQ_HI;
    bus_mode = BUS_SLAVE;

    // Clock the Z80 until it takes back control of the bus
    while (!GET_BUSACK)  {
        CLK_TOGGLE;
        if (i-- == 0) {
            printf_P(PSTR("bus master release timed out\n"));
            return;
        }
    }
}

/**
 * Retrieve the current bus status
 */
bus_stat bus_status(void)
{
    bus_stat status;
    uint16_t iox = iox_read16(0, GPIOA0);
    status.flags = (PINB & BMASK) | (PIND & DMASK);
    status.xflags = iox >> 8;
    status.data = DATA_PIN;
    status.addr = ADDRLO_PIN | ((iox & 0xFF) << 8);
    return status;
}

/**
 * Retrieve only the bus signals that can be retrieved quickly
 */
bus_stat bus_status_fast(void)
{
    bus_stat status;
    status.flags = (PINB & BMASK) | (PIND & DMASK);
    status.xflags = 0xFF;
    status.data = GET_DATA;
    status.addr = GET_ADDRLO;
    return status;
}

/**
 * Log the bus status
 */
void bus_log(bus_stat status)
{
    printf_P(
        PSTR("\t%04x %02x %c    %s %s    %s %s %s %s %s %s %s %s\n"),
        status.addr,
        status.data,
        0x20 <= status.data && status.data <= 0x7e ? status.data : ' ',
        !FLAG(status.flags, MREQ) ? "mreq" :
        !FLAG(status.flags, IORQ) ? "iorq " : "    ",
        !FLAG(status.flags, RD) ? "rd  " :
        !FLAG(status.flags, WR) ? "wr  " :
        !FLAG(status.xflags, RFSH) ? "rfsh" : "    ",
        !FLAG(status.xflags, M1) ? "m1" : "  ",
        !FLAG(status.flags, BUSRQ) ? "busrq" : "     ",
        !FLAG(status.flags, BUSACK) ? "busack" : "      ",
        (!FLAG(status.flags, IORQ) && FLAG(status.flags, BUSRQ)) ? "wait" : "    ",
        !FLAG(status.xflags, HALT) ? "halt" : "    ", 
        !FLAG(status.xflags, INTERRUPT) ? "int" : "   ",
        !FLAG(status.xflags, NMI) ? "nmi" : "   ",
        !FLAG(status.xflags, RESET) ? "reset" : "     ");
}

/**
 * Initialize the bus
 */
void bus_init(void)
{
    // Initialize I/O expander
    iox_init();

    // Configure bus signal direction
    RESET_INPUT;
    RESET_PULLUP;
    BUSRQ_OUTPUT;
    CLK_OUTPUT;

    IORQ_INPUT;
    BUSACK_INPUT;
    M1_INPUT;
    RFSH_INPUT;
    HALT_INPUT;

    // Pullup on halt so it can be used with a switch
    HALT_PULLUP;

    // Set defaults for output bus signals
    INT_HI;
    NMI_HI;
    BUSRQ_HI;

    // Reset the processor
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    BUSRQ_LO;
    BUSRQ_HI;

    // Start out in control of the bus
    bus_master();
}

/**
 * Output value to an IO register
 */
uint8_t io_out(uint8_t addr, uint8_t value)
{
    if (bus_mode != BUS_MASTER)
        return 0;
    MREQ_HI;
    RD_HI;
    _delay_us(10);
    SET_ADDRLO(addr);
    SET_DATA(value);
    DATA_OUTPUT;
    IORQ_LO;
    WR_LO;
    _delay_us(10);
    WR_HI;
    _delay_us(10);
    IORQ_HI;
    DATA_INPUT;
    return 1;
}

uint32_t base_addr = 0;

/**
 * Input value from an IO register
 */
uint8_t io_in(uint8_t addr)
{
    if (bus_mode != BUS_MASTER)
        return 0;
    MREQ_HI;
    WR_HI;
    _delay_ms(10);
    SET_ADDRLO(addr);
    DATA_INPUT;
    IORQ_LO;
    RD_LO;
    _delay_ms(10);
    uint8_t value = GET_DATA;
    RD_HI;
    IORQ_HI;
    return value;
}

#ifdef PAGE_BASE
#define PAGE_ENABLE (PAGE_BASE + 4)

uint8_t mem_pages[] = {0, 0, 0, 0};


/**
 * Set the specified memory bank to the specified page
 */
void mem_page(uint8_t bank, uint8_t page)
{
    bank &= 3;
    page &= 0x3f;
    uint8_t mreq = GET_MREQ;
    uint8_t rd = GET_RD;
    uint8_t dataddr = DATA_DDR;
    io_out(PAGE_ENABLE, 1);
    io_out(PAGE_BASE + bank, page);
    if (!mreq)
        MREQ_LO;
    if (!rd)
        RD_LO;
    DATA_DDR = dataddr;
    mem_pages[bank] = page;
}

/**
 * Bank in the page at the specified address
 */
void mem_page_addr(uint32_t addr)
{
    addr += base_addr;
    uint8_t page = (addr >> 14) & 0x3f;
    uint8_t bank = page & 3;
    mem_page(bank, page);
}
#endif

/**
 * Read specified number of bytes from external memory to a buffer
 */
uint8_t mem_read(uint32_t addr, void *buf, uint16_t len)
{
    uint8_t *cbuf = buf;
    if (bus_mode != BUS_MASTER)
        return 0;
    mem_page_addr(addr);
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr & 0xFFFF);
    for (uint16_t i = 0; i < len; i++) {
        cbuf[i] = GET_DATA;
        addr++;
        if ((addr & 0xFFFF) == 0)
            mem_page_addr(addr);
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr & 0xFFFF);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    RD_HI;
    MREQ_HI;
    return 1;
}

/**
 *  Write specified number of bytes to external memory from a buffer
 */
uint8_t _mem_write(uint32_t addr, const void *buf, uint16_t len, uint8_t pgmspace)
{
    const uint8_t *cbuf = buf;
    if (bus_mode != BUS_MASTER)
        return 0;
    mem_page_addr(addr);
    DATA_OUTPUT;
    MREQ_LO;
    SET_ADDR(addr & 0xFFFF);
    for (uint16_t i = 0; i < len; i++) {
        if (pgmspace)
            SET_DATA(pgm_read_byte(&cbuf[i]));
        else
            SET_DATA(cbuf[i]);
        WR_LO;
        WR_HI;
        addr++;
        if ((addr & 0xFFFF) == 0)
            mem_page_addr(addr);
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr & 0xFFFF);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    MREQ_HI;
    DATA_INPUT;
    return 1;
}

/**
 * Mute the SN76489
 */
void sn76489_mute()
{
    #ifdef SN76489_PORT
    if (bus_mode != BUS_MASTER)
        return;
    uint8_t oldclkdiv = clkdiv;
    clkdiv = 4;
    clk_run();
    DATA_OUTPUT;
    SET_ADDRLO(SN76489_PORT);
    IORQ_LO;
    SET_DATA(0x9F);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xBF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xDF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xFF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    IORQ_HI;
    clk_stop();
    clkdiv = oldclkdiv;
    #endif
}