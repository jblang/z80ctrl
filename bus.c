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

/**
 *  Request control of the bus from the Z80
 */
uint8_t bus_master(void)
{
    uint8_t i = 255;

    BUSRQ_LO;           // request bus
#ifdef IOACK_OUTPUT
    IOACK_LO;           // make sure not in WAIT state
    IOACK_HI;
#endif
    // wait for BUSACK to go low
    while (GET_BUSACK)  {
        CLK_TOGGLE;
        if (i-- == 0) {
            printf_P(PSTR("bus request timed out\n"));
            BUSRQ_HI;
            return 0;
        }
    }
    MREQ_HI;
    RD_HI;
    WR_HI;
    MREQ_OUTPUT;
#ifdef IORQ_OUTPUT
    IORQ_OUTPUT;
#endif
    RD_OUTPUT;
    WR_OUTPUT;
    ADDR_OUTPUT;
    DATA_INPUT;
    return 1;
}

/**
 *  Return control of the bus to the Z80
 */
void bus_slave(void)
{
    MREQ_INPUT;
#ifdef IORQ_INPUT
    IORQ_INPUT;
#endif
    RD_INPUT;
    WR_INPUT;
    ADDR_INPUT;
    DATA_INPUT;
    BUSRQ_HI;
}

/**
 * Retrieve the current bus status
 */
bus_stat bus_status()
{
    bus_stat status;
    status.flags = GET_BFLAGS | GET_DFLAGS;
    status.xflags = GET_XFLAGS;
    status.data = GET_DATA;
    status.addr = GET_ADDR;
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
#if (BOARD_REV < 3)
        !FLAG(status.xflags, MREQ) ? "memrq" :
#else
        !FLAG(status.flags, MREQ) ? "memrq" :
#endif
        !FLAG(status.flags, IORQ) ? "iorq " : "     ",

        !FLAG(status.flags, RD) ? "rd  " :
        !FLAG(status.flags, WR) ? "wr  " :
        !FLAG(status.xflags, RFSH) ? "rfsh" : "    ",
#if (BOARD_REV < 3)
        !FLAG(status.flags, M1) ? "m1" : "  ",
        !FLAG(status.xflags, BUSRQ) ? "busrq" : "     ",
        !FLAG(status.xflags, BUSACK) ? "busack" : "      ",
        (!FLAG(status.flags, IORQ) && FLAG(status.flags, IOACK)) ? "wait" : "    ",
        !FLAG(status.flags, HALT) ? "halt" : "    ", 
#else
        !FLAG(status.xflags, M1) ? "m1" : "  ",
        !FLAG(status.flags, BUSRQ) ? "busrq" : "     ",
        !FLAG(status.flags, BUSACK) ? "busack" : "      ",
        (!FLAG(status.flags, IORQ) && FLAG(status.flags, BUSRQ)) ? "wait" : "    ",
        !FLAG(status.xflags, HALT) ? "halt" : "    ", 
#endif    
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
#ifdef IOACK_OUTPUT
    IOACK_OUTPUT;
#endif
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
#ifdef IOACK_OUTPUT    
    IOACK_LO;
    IOACK_HI;
#else
    BUSRQ_LO;
    BUSRQ_HI;
#endif

    // Make bidirectional signals inputs
    bus_slave();
}

uint32_t base_addr = 0;

/**
 * Read specified number of bytes from external memory to a buffer
 */
void mem_read(uint32_t addr, uint8_t *buf, uint16_t len)
{
    addr += base_addr & 0xFC000;
    if (!bus_master())
        return;
#ifdef PAGE_BASE
    DATA_OUTPUT;
    mem_page_bare(0, PAGE(addr & 0xF0000));
    mem_page_bare(1, PAGE(addr & 0xF0000)+1);
    mem_page_bare(2, PAGE(addr & 0xF0000)+2);
    mem_page_bare(3, PAGE(addr & 0xF0000)+3);
#endif
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr & 0xFFFF);
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = GET_DATA;
        addr++;
#ifdef PAGE_BASE
        if ((addr & 0xFFFF) == 0) {
            MREQ_HI;
            DATA_OUTPUT;
            mem_page_bare(0, PAGE(addr));
            mem_page_bare(1, PAGE(addr)+1);
            mem_page_bare(2, PAGE(addr)+2);
            mem_page_bare(3, PAGE(addr)+3);
            DATA_INPUT;
            MREQ_LO;
        }
#endif
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr & 0xFFFF);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    RD_HI;
    MREQ_HI;
    bus_slave();
}

/**
 *  Write specified number of bytes to external memory from a buffer
 */
void _mem_write(uint32_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace)
{
    addr += base_addr & 0xFC000;
    if (!bus_master())
        return;
    DATA_OUTPUT;
#ifdef PAGE_BASE
    mem_page_bare(0, PAGE(addr & 0xF0000));
    mem_page_bare(1, PAGE(addr & 0xF0000)+1);
    mem_page_bare(2, PAGE(addr & 0xF0000)+2);
    mem_page_bare(3, PAGE(addr & 0xF0000)+3);
#endif
    MREQ_LO;
    SET_ADDR(addr & 0xFFFF);
    for (uint16_t i = 0; i < len; i++) {
        if (pgmspace)
            SET_DATA(pgm_read_byte(&buf[i]));
        else
            SET_DATA(buf[i]);
        WR_LO;
        WR_HI;
        addr++;
#ifdef PAGE_BASE
        if ((addr & 0xFFFF) == 0) {
            MREQ_HI;
            mem_page_bare(0, PAGE(addr));
            mem_page_bare(1, PAGE(addr)+1);
            mem_page_bare(2, PAGE(addr)+2);
            mem_page_bare(3, PAGE(addr)+3);
            MREQ_LO;
        }
#endif
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr & 0xFFFF);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    MREQ_HI;
    DATA_INPUT;
    bus_slave();
}

#ifdef IORQ_OUTPUT

/**
 * Output value to an IO register
 */
void io_out_bare(uint8_t addr, uint8_t value)
{
    SET_ADDRLO(addr);
    SET_DATA(value);
    IORQ_LO;
    WR_LO;
    WR_HI;
    IORQ_HI;
#ifdef IOACK_OUTPUT
    IOACK_LO;
    IOACK_HI;
#endif
}

void io_out(uint8_t addr, uint8_t value)
{
    if (!bus_master())
        return;
    DATA_OUTPUT;
    io_out_bare(addr, value);
    bus_slave();
}   

/**
 * Input value from an IO register
 */
uint8_t io_in_bare(uint8_t addr)
{
    SET_ADDRLO(addr);
    IORQ_LO;
    RD_LO;
    _delay_us(2);
    uint8_t value = GET_DATA;
    RD_HI;
    IORQ_HI;
#ifdef IOACK_OUTPUT
    IOACK_LO;
    IOACK_HI;
#endif
    return value;
}

uint8_t io_in(uint8_t addr)
{
    if (!bus_master())
        return 0;
    DATA_INPUT;
    uint8_t value = io_in_bare(addr);
    bus_slave();
    return value;
}

void sn76489_mute()
{
    #ifdef SN76489_PORT
    if (!bus_master())
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
    bus_slave();
    #endif
}

#endif

#ifdef PAGE_BASE
#define PAGE_ENABLE (PAGE_BASE + 4)     // enable paging

uint8_t mem_pages[] = {0, 0, 0, 0};

void mem_page_bare(uint8_t bank, uint8_t page)
{
    io_out_bare(PAGE_ENABLE, 1);
    io_out_bare(PAGE_BASE + (bank & 3), page & 0x3f);
}

/**
 * Select the specified memory page for specified bank
 */
void mem_page(uint8_t bank, uint8_t page)
{
    if (!bus_master())
        return;
    DATA_OUTPUT;
    mem_page_bare(bank, page);
    bus_slave();
}

#endif