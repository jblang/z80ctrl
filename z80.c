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
#include "util.h"
#ifdef TMS_BASE
#include "tms.h"
#endif

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
 * Whether to stop when the halt signal occurs
 */
uint8_t do_halt = 1;
uint8_t halt_key = 0;

/**
 * Reset the Z80 to a specified address
 */
void z80_reset(uint32_t addr)
{
    sn76489_mute();
    uint8_t reset_vect[] = { 0xC3, (addr & 0xFF), ((addr >> 8) & 0xFF) };
    if (addr > 0x0002) {
        mem_write(0x0000, reset_vect, 3);
    }
    bus_request();
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    bus_release();
#ifdef PAGE_BASE
    for (uint8_t i = 0; i < 4; i++)
        mem_page(i, (base_addr >> 14) + i);
#endif
#ifdef TMS_BASE
    tms_config();
#endif
}

/**
 * Run the Z80 at full speed
 */
void z80_run(void)
{
    bus_release();
    clk_run();
    for(;;) {
        if (uart_peek(0) == halt_key && halt_key != 0)
            break;
        if (!GET_IORQ)
            iorq_dispatch(0);
        if (do_halt && !GET_HALT)
            break;
    }
    clk_stop();
    CLK_LO;
    bus_request();
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

    CLK_LO;
    CLK_HI;

    if (!GET_MREQ && (ENABLED(watches, MEMRD) || ENABLED(breaks, MEMRD) || 
                     ENABLED(watches, MEMWR) || ENABLED(breaks, MEMWR))) {
        if (lastrd && !GET_RD) {
            bus_stat status = bus_status();
            if (logged = INRANGE(watches, MEMRD, status.addr)) {
                bus_log(status);
                uart_flush();
            }
            if (INRANGE(breaks, MEMRD, status.addr)) {
                printf_P(PSTR("memrd break at %04X\n"), status.addr);
                uart_flush();
                return 1;
            }
        } else if (lastwr && !GET_WR) {
            bus_stat status = bus_status();
            if (logged = INRANGE(watches, MEMWR, status.addr)) {
                bus_log(status);
                uart_flush();
            }
            if (INRANGE(breaks, MEMWR, status.addr)) {
                printf_P(PSTR("memwr break at %04X\n"), status.addr);
                uart_flush();
                return 1;
            };
        }
    } 
    
    if (!GET_IORQ) {
        if (lastrd && !GET_RD) {
            logged = INRANGE(watches, IORD, GET_ADDRLO);
            if (INRANGE(breaks, IORD, GET_ADDRLO)) {
                printf_P(PSTR("iord break at %02x\n"), GET_ADDRLO);
                uart_flush();
                return 1;
            }
        } else if (lastwr && !GET_WR) {
            logged = INRANGE(watches, IOWR, GET_ADDRLO);
            if (INRANGE(breaks, IOWR, GET_ADDRLO)) {
                printf_P(PSTR("iowr break at %02x\n"), GET_ADDRLO);
                uart_flush();
                return 1;
            }
        }
    }

    if (ENABLED(watches, BUS) && !logged) {
        bus_stat status = bus_status();
        if (INRANGE(watches, BUS, status.addr)) {
            bus_log(status);
            uart_flush();
        }
    }

    if (!GET_IORQ) {
        set_tcnt(1, 0);
        iorq_dispatch(logged);
        if (logged) {
            uint16_t tcnt = get_tcnt(1);
            printf_P(PSTR("\t%d us"), TCNT_TO_US(tcnt, F_CPU));
            bus_log(iorq_stat);
            uart_flush();
        }
    }
        
    return 0;
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
    config_timer(1, CLKDIV1);
    
    bus_release();
    while (GET_HALT && (cycles == 0 || c < cycles)) {
        if (uart_peek(0) == halt_key && halt_key != 0)
            break;
        if ((ENABLED(watches, OPFETCH) || ENABLED(breaks, OPFETCH) || cycles)) {
            if (!GET_RD && !GET_MREQ && !GET_M1) {
                uint16_t addr = GET_ADDR;
                if (INRANGE(breaks, OPFETCH, addr) && !cycles && !brkonce) {
                    printf_P(PSTR("opfetch break at %04x\n"), addr);
                    brkonce = 1;
                    break;
                }
                brkonce = 0;
                disasmbrk = 0;
                disasm(z80_read, mnemonic);
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
    bus_request();
    config_timer(1, CLKOFF);
}