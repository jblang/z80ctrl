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
#include <avr/cpufunc.h>
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

/**
 *  Request control of the bus from the Z80
 */
uint8_t bus_request(void)
{
    uint8_t i = 255;

    BUSRQ_LO;           // request bus
    // Clock the Z80 until it releases control of the bus
    while (GET_BUSACK)  {
        CLK_TOGGLE;
        if (i-- == 0) {
            printf_P(PSTR("bus request timed out\n"));
            BUSRQ_HI;
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
    return 1;
}

/**
 *  Return control of the bus to the Z80
 */
void bus_release(void)
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

    // Clock the Z80 until it takes back control of the bus
    while (!GET_BUSACK)  {
        CLK_TOGGLE;
        if (i-- == 0) {
            printf_P(PSTR("bus release timed out\n"));
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
    uint16_t iox = iox0_read16(ADDRHI_GPIO);
    status.flags = (PINB & CTRLB_MASK) | (PIND & CTRLD_MASK);
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
    status.flags = (PINB & CTRLB_MASK) | (PIND & CTRLD_MASK);
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
        !MREQ_STATUS ? "mreq" :
        !IORQ_STATUS ? "iorq " : "    ",
        !RD_STATUS ? "rd  " :
        !WR_STATUS ? "wr  " :
        !RFSH_STATUS ? "rfsh" : "    ",
        !M1_STATUS ? "m1" : "  ",
        !BUSRQ_STATUS ? "busrq" : "     ",
        !BUSACK_STATUS ? "busack" : "      ",
        !WAIT_STATUS ? "wait" : "    ",
        !HALT_STATUS ? "halt" : "    ", 
        !INT_STATUS ? "int" : "   ",
        !NMI_STATUS ? "nmi" : "   ",
        !RESET_STATUS ? "reset" : "     ");
}

/**
 * Initialize the bus
 */
void bus_init(void)
{
    // Initialize I/O expander
    iox_init();

    // Initialize control signals
    CTRLB_OUTPUT_INIT;
    CTRLD_OUTPUT_INIT;
    CTRLX_OUTPUT_INIT;
    CTRLX_PULLUP_INIT;

    // Reset the processor
    BUSRQ_HI;
    RESET_OUTPUT;
    clk_cycle(3);
    RESET_INPUT;

    // Start out in control of the bus
    bus_request();
}

/**
 * Output value to an IO register
 */
uint8_t io_out(uint8_t addr, uint8_t value)
{
    if (GET_BUSACK)
        return 0;
    MREQ_HI;
    RD_HI;
    _delay_us(1);
    SET_ADDRLO(addr);
    SET_DATA(value);
    DATA_OUTPUT;
    IORQ_LO;
    WR_LO;
    _delay_us(1);
    WR_HI;
    _delay_us(1);
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
    if (GET_BUSACK)
        return 0;
    MREQ_HI;
    WR_HI;
    _delay_us(1);
    SET_ADDRLO(addr);
    DATA_INPUT;
    IORQ_LO;
    RD_LO;
    _delay_us(1);
    uint8_t value = GET_DATA;
    RD_HI;
    IORQ_HI;
    return value;
}

/**
 * Bank in the page at the specified address
 */
#ifdef BANK_PORT
uint8_t mem_banks = 0;

void mem_bank_addr(uint32_t addr)
{
    mem_banks = (addr >> 15) & 0x0e;
    mem_banks |= (mem_banks + 1) << 4;
    io_out(BANK_PORT, mem_banks);
}
#elif defined(BANK_BASE)
#define BANK_ENABLE (BANK_BASE + 4)
uint8_t mem_banks[] = {0, 0, 0, 0};
void mem_bank(uint8_t bank, uint8_t addr)
{
    bank &= 3;
    addr &= 0x3f;
    io_out(BANK_ENABLE, 1);
    io_out(BANK_BASE + bank, addr);
    mem_banks[bank] = addr;
}

void mem_bank_addr(uint32_t addr)
{
    uint8_t page = (addr >> 14) & 0x3c;
    for (uint8_t i = 0; i < 4; i++)
        mem_bank(i, page + i);
}
#endif

void mem_read_page(uint8_t start, uint8_t end, void *buf)
{
    if (GET_BUSACK)
        return;
    uint8_t *bufbyte = buf;
    DATA_INPUT;
    RD_LO;
    do {
        SET_ADDRLO(start++);
        _NOP();
        _NOP();
        *bufbyte++ = GET_DATA;
    } while (start <= end && start != 0);
    RD_HI;
}

void mem_write_page(uint8_t start, uint8_t end, void *buf)
{
    if (GET_BUSACK)
        return;
    uint8_t *bufbyte = buf;
    DATA_OUTPUT;
    do {
        SET_ADDRLO(start++);
        SET_DATA(*bufbyte++);
        WR_LO;
        WR_HI;
    } while (start <= end && start != 0);
    DATA_INPUT;
}

void mem_write_page_P(uint8_t start, uint8_t end, void *buf)
{
    if (GET_BUSACK)
        return;
    uint8_t *bufbyte = buf;
    DATA_OUTPUT;
    do {
        SET_ADDRLO(start++);
        SET_DATA(pgm_read_byte(bufbyte++));
        WR_LO;
        WR_HI;
    } while (start <= end && start != 0);
    DATA_INPUT;
}

void mem_iterate(uint16_t start, uint16_t end, pagefunc_t dopage, void *buf)
{
    if (GET_BUSACK)
        return;
    uint8_t starthi = start >> 8;
    uint8_t startlo = start & 0xff;
    uint8_t endhi = end >> 8;
    uint8_t endlo = end & 0xff;
    MREQ_LO;
    SET_ADDRHI(starthi);
    if (starthi == endhi) {
        dopage(startlo, endlo, buf);
    } else {
        dopage(startlo, 0xff, buf);
        buf += 0x100 - startlo;
        starthi++;
        while (starthi < endhi) {
            SET_ADDRHI(starthi++);
            dopage(0, 0xff, buf);
            buf += 0x100;
        }
        SET_ADDRHI(starthi);
        dopage(0, endlo, buf);
    }
    MREQ_HI;
}

void mem_iterate_banked(uint32_t start, uint32_t end, pagefunc_t dopage, void *buf)
{
    if (GET_BUSACK)
        return;
    start += base_addr;
    end += base_addr;
    mem_bank_addr(start);
    if ((start & 0x10000) == (end & 0x10000)) {
        mem_iterate(start & 0xffff, end & 0xffff, dopage, buf);
    } else {
        mem_iterate(start & 0xffff, 0xffff, dopage, buf);
        buf += 0x10000 - start;
        mem_bank_addr(end);
        mem_iterate(0, end & 0xffff, dopage, buf);
    }
}

/**
 * Mute the SN76489
 */
void sn76489_mute()
{
    #ifdef SN76489_PORT
    if (GET_BUSACK)
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