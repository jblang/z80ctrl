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
 * @file z80.c Z80 run loop and debugger
 */

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "disasm.h"
#include "uart.h"
#include "iorq.h"

/**
 * Breakpoints and watch names
 */
const char debug_names[] PROGMEM = "memrd\0memwr\0iord\0iowr\0opfetch\0bus";

/**
 * Breakpoint and watch ranges
 */
range breaks[] = {{0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}};
range watches[] = {{0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}, {0xffff, 0}};

/**
 * Reset the Z80 to a specified address
 */
void z80_reset(uint16_t addr)
{
    uint8_t reset_vect[] = { 0xc3, (addr & 0xFF), (addr >> 8) };
    if (addr > 0x0002) {
        write_mem(0x0000, reset_vect, 3);
    }
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
#ifdef PAGE_BASE
    mem_restore();
#endif
#ifdef IOACK_OUTPUT
    IOACK_LO;
    IOACK_HI;
#else
    BUSRQ_LO;
    BUSRQ_HI;
#endif
}

/**
 * Run the Z80 at full speed
 */
void z80_run(void)
{
    uint16_t count;
    clk_run();
    for(;;) {
        if (!GET_IORQ)
            iorq_dispatch();
        if (++count == 0 && !GET_HALT)
            break;
    }
    clk_stop();
    CLK_LO;
}

/**
 * Log the bus status
 */
void z80_buslog(bus_stat status)
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

        // wait until output is fully transmitted to avoid
        // interfering with z80_uart status for running program
        uart_flush();
}

/**
 * Do a single T cycle, optionally logging and breaking on the bus status
 */
uint8_t z80_tick()
{
    // Save previous RD and WR values to detect falling edge
    uint8_t lastrd = GET_RD;
    uint8_t lastwr = GET_WR;

    uint8_t logged = 0;
    uint8_t brk = 0;

    CLK_LO;
    CLK_HI;

    if (GET_IORQ && (ENABLED(watches, MEMRD) || ENABLED(breaks, MEMRD) || 
                     ENABLED(watches, MEMWR) || ENABLED(breaks, MEMWR))) {
        if (lastrd && !GET_RD) {
            bus_stat status = bus_status();
            if (!FLAG(status.xflags, MREQ)) {
                if (logged = INRANGE(watches, MEMRD, status.addr))
                    z80_buslog(status);
                if (INRANGE(breaks, MEMRD, status.addr)) {
                    printf_P(PSTR("memrd break at %04X\n"), status.addr);
                    uart_flush();
                    brk = 1;
                }
            }
        } else if (lastwr && !GET_WR) {
            bus_stat status = bus_status();
            if (!FLAG(status.xflags, MREQ)) {
                if (logged = INRANGE(watches, MEMWR, status.addr))
                    z80_buslog(status);
                if (INRANGE(breaks, MEMWR, status.addr)) {
                    printf_P(PSTR("memwr break at %04X\n"), status.addr);
                    uart_flush();
                    brk = 1;
                };
            }
        }
    } 
    
    if (!GET_IORQ) {
        if (lastrd && !GET_RD) {
            if (logged = INRANGE(watches, IORD, GET_ADDRLO)) {
                bus_stat status = bus_status();
                z80_buslog(status);
            }
            if (INRANGE(breaks, IORD, GET_ADDRLO)) {
                printf_P(PSTR("iord break at %02X\n"), GET_ADDRLO);
                uart_flush();
                brk = 1;
            }
        } else if (lastwr && !GET_WR) {
            if (logged = INRANGE(watches, IOWR, GET_ADDRLO)) {
                bus_stat status = bus_status();
                z80_buslog(status);
            }
            if (INRANGE(breaks, IOWR, GET_ADDRLO)) {
                printf_P(PSTR("iowr break at %02X\n"), GET_ADDRLO);
                uart_flush();
                brk = 1;
            }
        }
    }

    if (ENABLED(watches, BUS) && !logged) {
        bus_stat status = bus_status();
        if (INRANGE(watches, BUS, status.addr))
            z80_buslog(status);
    }

    if (!GET_IORQ)
        iorq_dispatch();
        
    return brk;
}

uint8_t disasmbrk = 0;

/**
 * Clock the Z80 until it completes a memory read cycle and return the value read
 */
uint8_t z80_read()
{
    uint8_t data;
    while (GET_RD || GET_MREQ)
        disasmbrk |= z80_tick();
    data = GET_DATA;
    while (!GET_RD && !GET_MREQ)
        disasmbrk |= z80_tick();
    return data;
}

/**
 * Run the Z80 with watches and breakpoints for a specified number of instructions
 */
void z80_debug(uint32_t cycles)
{
    char mnemonic[255];
    uint32_t c = 0;
    static uint8_t brkonce = 0;

    while (GET_HALT && (cycles == 0 || c < cycles)) {
        if ((ENABLED(watches, OPFETCH) || ENABLED(breaks, OPFETCH) || cycles)) {
            if (!GET_RD && !GET_MREQ && !GET_M1) {
                uint16_t addr = GET_ADDR;
                if (INRANGE(breaks, OPFETCH, addr) && !cycles && !brkonce) {
                    printf_P(PSTR("opfetch break at %04X\n"), addr);
                    brkonce = 1;
                    break;
                }
                brkonce = 0;
                disasmbrk = 0;
                disasm(addr, z80_read, mnemonic);
                if (INRANGE(watches, OPFETCH, addr)) {
                    printf_P(PSTR("\t%04x\t%s\n"), addr, mnemonic);
                    uart_flush();
                }
                c++;
                if (disasmbrk && !cycles)
                    break;
            }
        }
        if(z80_tick())
            break;
    }
}