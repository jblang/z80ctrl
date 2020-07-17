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
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    bus_request();
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
            iorq_dispatch();
        if (do_halt && !GET_HALT)
            break;
    }
    clk_stop();
    CLK_LO;
    bus_request();
}

#define MULTIBYTE(op) ((op) == 0xCB || (op) == 0xDD || (op) == 0xED || (op) == 0xFD);

/**
 * Run the Z80 with watches and breakpoints for a specified number of instructions
 */
void z80_debug(uint32_t cycles)
{
    uint8_t last_rd;
    uint8_t last_wr;
    static uint8_t ignore_m1;

    if (cycles > 0)
        cycles--;   // 0 based counter makes comparison cheaper

    bus_release();
    for (;;) {
        last_rd = GET_RD;
        last_wr = GET_WR;
        CLK_HI;
        CLK_LO;
        if ((last_rd && !GET_RD) || (last_wr && !GET_WR)) { // falling edge
            bus_stat status = bus_status();
            if (!FLAG(status.xflags, HALT) && do_halt)
                break;
            if (!FLAG(status.flags, IORQ)) {
                status.data = iorq_dispatch();
                if (!FLAG(status.flags, RD)) {
                    if (INRANGE(watches, IORD, status.addr & 0xff)) {
                        bus_log(status);
                        uart_flush();
                    }
                    if (INRANGE(breaks, IORD, status.addr & 0xff) && cycles-- == 0) {
                        printf_P(PSTR("iord break at %04x\n"), status.addr);
                        break;
                    }
                } else { // IORQ WR
                    if (INRANGE(watches, IOWR, status.addr & 0xff)) {
                        bus_log(status);
                        uart_flush();
                    }
                    if (INRANGE(breaks, IOWR, status.addr & 0xff) && cycles-- == 0) {
                        printf_P(PSTR("iowr break at %04x\n"), status.addr);
                        break;
                    }
                }
            } else { // MREQ
                if (!FLAG(status.flags, RD)) {
                    if (INRANGE(watches, MEMRD, status.addr)) {
                        bus_log(status);
                        uart_flush();
                    }
                    if (!FLAG(status.xflags, M1)) {
                        if (!ignore_m1) {
                            if (INRANGE(watches, OPFETCH, status.addr)) {
                                bus_request();
                                disasm_mem(status.addr, status.addr);
                                uart_flush();
                            }
                            if (INRANGE(breaks, OPFETCH, status.addr) && cycles-- == 0) {
                                printf_P(PSTR("opfetch break at %04x\n"), status.addr);
                                ignore_m1 = MULTIBYTE(status.data);
                                break;
                            }
                            if (INRANGE(watches, OPFETCH, status.addr))
                                bus_release();
                        }
                        ignore_m1 = MULTIBYTE(status.data);
                    }
                    if (INRANGE(breaks, MEMRD, status.addr) && cycles-- == 0) {
                        printf_P(PSTR("memrd break at %04x\n"), status.addr);
                        break;
                    }
                } else { // MREQ WR
                    if (INRANGE(watches, MEMWR, status.addr)) {
                        bus_log(status);
                        uart_flush();
                    }
                    if (INRANGE(breaks, MEMWR, status.addr) && cycles-- == 0) {
                        printf_P(PSTR("memwr break at %04x\n"), status.addr);
                        break;
                    }
                }
            }
        }
    }
    bus_request();
}