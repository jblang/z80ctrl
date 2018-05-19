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

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "diskemu.h"
#include "disasm.h"
#include "memory.h"
#include "uart.h"

// Breakpoints and watches
const char debug_names[] PROGMEM = "memrd\0memwr\0iord\0iowr\0opfetch\0bus";

range breaks[] = {{0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}};
range watches[] = {{0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}};

// Reset the Z80
void z80_reset(uint16_t addr)
{
    uint8_t reset_vect[] = { 0xc3, (addr & 0xFF), (addr >> 8) };
    if (addr > 0x0002) {
        write_mem(0x0000, reset_vect, 3);
    }
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    IOACK_LO;
    IOACK_HI;
}

#define SIO0_STATUS 0x10
#define SIO0_DATA 0x11
#define SIOA_CONTROL 0x80
#define SIOA_DATA 0x81
#define SIOB_CONTROL 0x82
#define SIOB_DATA 0x83

// Handle Z80 IO request
void z80_iorq(void)
{
    switch (GET_ADDRLO) {
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA((((uart_testtx() == 0) << 1) & 0x2) | ((uart_testrx() > 0) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIOA_CONTROL:
            if (!GET_RD) {
                // CTS and DCD always high
                SET_DATA((1 << 3) | (1  << 5) | (((uart_testtx() == 0) << 2) & 0x4) | ((uart_testrx() > 0)& 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIOB_CONTROL:
            if (!GET_RD) {
                // CTS and DCD always high
                SET_DATA((1 << 3) | (1  << 5) | (((uart_testtx() == 0) << 2) & 0x4) | ((uart_testrx() > 0) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIO0_DATA:
        case SIOA_DATA:
            if (!GET_RD) {
                SET_DATA(uart_getc());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                uart_putc(GET_DATA);
            }
            break;
        case SIOB_DATA:
            if (!GET_RD) {
                SET_DATA(uart_getc());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                uart_putc(GET_DATA);
            }
            break;
        case DRIVE_STATUS:
            if (!GET_RD) {
                SET_DATA(drive_status());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_select(GET_DATA);
            }
            break;
        case DRIVE_CONTROL:
            if (!GET_RD) {
                SET_DATA(drive_sector());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_control(GET_DATA);
            }
            break;
        case DRIVE_DATA:
            if (!GET_RD) {
                SET_DATA(drive_read());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_write(GET_DATA);
            }
            break;
        default:
            if (!GET_RD) {
                SET_DATA(0xFF);
            }
    }
    if (!GET_RD)
        BUSRQ_LO;
    IOACK_LO;
    while (!GET_IORQ) {
        CLK_TOGGLE;
    }
    DATA_INPUT;
    IOACK_HI;
    BUSRQ_HI;
}

// Run the Z80 at full speed
void z80_run(void)
{
    clk_run();
    while (GET_HALT) {
        if (!GET_IORQ) {
            z80_iorq();
        }
    }
    clk_stop();
    CLK_LO;
}


// Log the bus status
void z80_buslog(bus_stat status)
{
    printf_P(
        PSTR("\t%04x %02x %c    %s %s    %s %s %s %s %s %s %s %s\n"),
        status.addr,
        status.data,
        0x20 <= status.data && status.data <= 0x7e ? status.data : ' ',
        !status.flags.bits.rd ? "rd  " :
        !status.flags.bits.wr ? "wr  " :
        !status.flags.bits.rfsh ? "rfsh" : "    ",
        !status.flags.bits.mreq ? "mem" :
        !status.flags.bits.iorq ? "io " : "   ",
        !status.flags.bits.m1 ? "m1" : "  ",
        !status.flags.bits.halt ? "halt" : "    ", 
        !status.flags.bits.interrupt ? "int" : "   ",
        !status.flags.bits.nmi ? "nmi" : "   ",
        !status.flags.bits.reset ? "rst" : "   ",
        !status.flags.bits.busrq ? "busrq" : "     ",
        !status.flags.bits.busack ? "busack" : "      ",
        !status.flags.bits.ioack ? "ioack" : "     ");

        // wait until output is fully transmitted to avoid
        // interfering with UART status for running program
        while (uart_testtx())
            ;
}

// Do a single T cycle, optionally logging and breaking on the bus status
uint8_t z80_tick()
{
    // Save previous RD and WR values to detect falling edge
    uint8_t lastrd = GET_RD;
    uint8_t lastwr = GET_WR;

    uint8_t logged = 0;
    uint8_t brk = 0;

    CLK_LO;
    CLK_HI;

    // Only do expensive IO-expander read if memory logging or breakpoint is enabled
    if (ENABLED(watches, MEMRD) || ENABLED(breaks, MEMRD)) {
        bus_stat status = bus_status();
        if (!status.flags.bits.mreq)
            // Only log memory access once, on the falling edge of RD or WR
            if (lastrd && !GET_RD) {
                if (logged = INRANGE(watches, MEMRD, status.addr))
                    z80_buslog(status);
                brk = INRANGE(breaks, MEMRD, status.addr);
            } else if (lastwr && !GET_WR) {
                if (logged = INRANGE(watches, MEMWR, status.addr))
                    z80_buslog(status);
                brk = INRANGE(breaks, MEMWR, status.addr);
            }
    } 
    
    // Handle IO requests, and optionally log or break on them
    if (!GET_IORQ) {
        if (lastrd && !GET_RD) {
            if (logged = INRANGE(watches, IORD, GET_ADDRLO)) {
                bus_stat status = bus_status();
                z80_buslog(status);
            }
            brk = INRANGE(breaks, IORD, GET_ADDRLO);
        } else if (lastwr && !GET_WR) {
            if (logged = INRANGE(watches, IOWR, GET_ADDRLO)) {
                bus_stat status = bus_status();
                z80_buslog(status);
            }
            brk = INRANGE(breaks, IOWR, GET_ADDRLO);
        }
        z80_iorq();
    }

    // Log if bus logging is enabled and this cycle wasn't already logged for another reason
    if (ENABLED(watches, BUS) && !logged) {
        bus_stat status = bus_status();
        if (INRANGE(watches, BUS, status.addr))
            z80_buslog(status);
    }
    return brk;
}

// Clock the Z80 until it completes a memory read cycle and return the value read
uint8_t z80_read()
{
    uint8_t data;
    // Wait for read cycle to start
    while (GET_MREQ || GET_RD)
        z80_tick();
    // Capture value read
    data = GET_DATA;
    // Wait for read cycle to end
    while (!GET_MREQ || !GET_RD)
        z80_tick();
    return data;
}

// Run the Z80 with watches and breakpoints for a specified number of instructions
void z80_debug(uint32_t cycles)
{
    char mnemonic[255];
    uint32_t c = 0;
    uint8_t brk = 0;

    while (GET_HALT && (cycles == 0 || c < cycles) && !brk) {
        brk = z80_tick();
        // Only do expensive stuff if opcode break or watch is enabled
        if (ENABLED(watches, OPFETCH) || ENABLED(breaks, OPFETCH) || cycles)
            if (!GET_M1) {
                // Check if we are at a breakpoint
                uint16_t addr = GET_ADDR;
                // If opcode is being fetched, disassemble it to fetch the rest of the instruction
                if (!GET_MREQ && !GET_RD) {
                    c++;
                    disasm(addr, z80_read, mnemonic);
                    // If opcode was within watch range, log the instruction
                    if (INRANGE(watches, OPFETCH, addr)) {
                        printf_P(PSTR("\t%04x\t%s\n"), addr, mnemonic);
                        // Flush UART to avoid interfering with running program
                        while (uart_testtx())
                            ;
                    }
                }
                brk |= !GET_M1 && INRANGE(breaks, OPFETCH, addr) && !cycles;
            }
    }
}