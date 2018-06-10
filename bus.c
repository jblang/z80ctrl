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

uint8_t clkdiv = 1;

/**
 * Run the Z80's clock
 */
void clk_run(void)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = clkdiv;
    OCR2B = clkdiv >> 1;
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
 * Initialize the bus
 */
void bus_init(void)
{
    // Initialize I/O expander
    iox_init();

    // Configure bus signal direction
    INT_OUTPUT;
    NMI_OUTPUT;
    RESET_OUTPUT;
    BUSRQ_OUTPUT;
#ifdef BANK_OUTPUT
    BANK_OUTPUT;
#endif
#ifdef IOACK_OUTPUT
    IOACK_OUTPUT;
#endif
    CLK_OUTPUT;

    IORQ_INPUT;
    BUSACK_INPUT;
    M1_INPUT;
#ifdef RFSH_INPUT
    RFSH_INPUT;
#endif
    HALT_INPUT;

    // Pullup on halt so it can be used with a switch
    HALT_PULLUP;

    // Set defaults for output bus signals
    INT_HI;
    NMI_HI;
    BUSRQ_HI;
#ifdef SET_BANK
    SET_BANK(0);
#endif

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

/**
 * Read specified number of bytes from external memory to a buffer
 */
void read_mem(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if(!bus_master())
        return;
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr);
    for (i = 0; i < len; i++) {
        buf[i] = GET_DATA;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
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
void _write_mem(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace)
{
    if(!bus_master())
        return;
    DATA_OUTPUT;
    MREQ_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        if (pgmspace)
            SET_DATA(pgm_read_byte(&buf[i]));
        else
            SET_DATA(buf[i]);
        WR_LO;
        WR_HI;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
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
    DATA_OUTPUT;
    IORQ_OUTPUT;
    IORQ_LO;
    WR_LO;
    WR_HI;
    IORQ_HI;
    IORQ_INPUT;
#ifdef IOACK_OUTPUT
    IOACK_LO;
    IOACK_HI;
#endif
}

void io_out(uint8_t addr, uint8_t value)
{
    if (!bus_master())
        return;
    io_out_bare(addr, value);
    bus_slave();
}   

/**
 * Input value from an IO register
 */
uint8_t io_in_bare(uint8_t addr)
{
    SET_ADDRLO(addr);
    IORQ_OUTPUT;
    IORQ_LO;
    RD_LO;
    uint8_t value = GET_DATA;
    RD_HI;
    IORQ_HI;
    IORQ_INPUT;
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
    uint8_t value = io_in_bare(addr);
    bus_slave();
    return value;
}   


#define PAGE(addr) ((addr >> 14) & 0x3f)
#define PAGE_BASE 0x28                  // base address of page registers
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
    mem_pages[bank & 3] = page;               // save pages so they can be restored after reset
    if(!bus_master())
        return;
    mem_page_bare(bank, page);
    bus_slave();
}

/**
 * Restore previous paging configuration
 */
void mem_restore(void)
{
    for (uint8_t i = 0; i < 4; i++)
        mem_page(i, mem_pages[i]);
}

// Reference: http://ww1.microchip.com/downloads/en/DeviceDoc/20005022C.pdf

void flash_cmd_prefix(void)
{
        SET_ADDR(0x5555);
        SET_DATA(0xAA);
        WR_LO;
        WR_HI;
        SET_ADDR(0x2AAA);
        SET_DATA(0x55);
        WR_LO;
        WR_HI;
}


void flash_write(uint32_t addr, uint8_t *buf, uint16_t len)
{
    if(!bus_master())
        return;
    // first two banks must be physical pages 0 and 1
    mem_page_bare(0, 0);
    mem_page_bare(1, 1);
    mem_page_bare(2, PAGE(addr));
    DATA_OUTPUT;
    MREQ_LO;
    for (uint16_t i = 0; i < len; i++) {
        // Send byte program command sequence
        flash_cmd_prefix();
        SET_ADDR(0x5555);
        SET_DATA(0xA0);
        WR_LO;
        WR_HI;

        // Write byte
        if ((addr & 0x3fff) == 0) {
            // Load page to write into bank 3
            MREQ_HI;
            mem_page_bare(2, PAGE(addr));
            MREQ_LO;
        }
        SET_ADDR((addr & 0x3fff) + 0x8000);
        SET_DATA(buf[i]);
        WR_LO;
        WR_HI;

        // Wait for write to finish
        _delay_us(20);
        addr++;
    }
    MREQ_HI;
    DATA_INPUT;
    bus_slave();
    mem_restore();
}


/** 
 * Erase the 4KB sector that the address falls within
 */
void flash_erase(uint32_t addr)
{
    if(!bus_master())
        return;
    // first two banks must be physical pages 0 and 1
    mem_page_bare(0, 0);
    mem_page_bare(1, 1);
    DATA_OUTPUT;
    MREQ_LO;
    // Send erase command sequence
    flash_cmd_prefix();
    SET_ADDR(0x5555);
    SET_DATA(0x80);
    WR_LO;
    WR_HI;
    flash_cmd_prefix();
    if (addr > 0x7FFFF) {
        // Erase entire chip
        SET_ADDR(0x5555);
        SET_DATA(0x10);
        WR_LO;
        WR_HI;
        _delay_ms(100);
    } else {
        // Erase 4KB sector
        MREQ_HI;
        mem_page_bare(0, PAGE(addr));
        MREQ_LO;
        SET_ADDR(addr & 0x3000);
        SET_DATA(0x30);
        WR_LO;
        WR_HI;
        _delay_ms(100);
    }
    MREQ_HI;
    DATA_INPUT;
    bus_slave();
    mem_restore();
}
#endif