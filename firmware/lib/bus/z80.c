/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
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

#include "disasm.h"
#include "harvard.h"

#include "bus.h"
#include "io.h"
#include "memory.h"
#include "sn76489.h"
#include "avr_timer.h"
#ifdef TMS_BASE
#include "tms9918.h"
#endif
#include "avr_uart.h"
#include "z80.h"

/**
 * Breakpoints and watch names
 */
const char debug_names[] PROGMEM = "memrd\0memwr\0iord\0iowr\0opfetch\0bus";

/**
 * Breakpoint and watch ranges
 */
range breaks[] = { { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 } };
range watches[] = { { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 }, { 0xffff, 0 } };

/**
 * Whether to stop when the halt signal occurs
 */
uint8_t halt_mask = RESET | HALT;

/**
 * Set the halt mask
 */
void set_haltmask(uint8_t mask)
{
    halt_mask = mask;
}

/**
 * Get the halt maks
 */
uint8_t get_haltmask(void)
{
    return halt_mask;
}

/**
 * Reset the Z80 to a specified address
 */
void z80_reset(uint32_t addr)
{
    sn76489_mute();
    uint8_t reset_vect[] = { 0xC3, (addr & 0xFF), ((addr >> 8) & 0xFF) };
    if (addr > 0x0002) {
        mem_write_banked(0x0000, reset_vect, 3);
    }
    bus_release();
    RESET_OUTPUT;
    clk_cycle(3);
    RESET_INPUT;
    if (!bus_request())
        printf_P(PSTR("bus request timed out"));
#if defined(BANK_PORT) || defined(BANK_BASE)
    mem_bank_addr(base_addr);
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
    // configure IO extender to interrupt on halt condition
    iox0_write(IOX_INTCON(CTRLX_PORT), 0xff);
    iox0_write(IOX_DEFVAL(CTRLX_PORT), 0xff);
    iox0_write(IOX_GPINTEN(CTRLX_PORT), halt_mask);
    iox0_read(IOX_INTCAP(CTRLX_PORT)); // clear interrupts
    watch_flag = 0;
    bus_release();
    clk_run();
    for (;;) {
        if (!GET_WAIT)
            iorq_dispatch();
        if (watch_flag) {
            printf_P(PSTR("breaking due to halt key\n"));
            break;
        }
        if (!halt_mask) // halt checking disabled
            continue;
#if (BOARD_REV == 5 || BOARD_REV == 6)
        // skip slow status check if no interrupt from IO extender
        if (GET_IOXINT)
            continue;
#endif
        uint8_t xflags = iox0_read(IOX_INTCAP(CTRLX_PORT));
        if ((xflags & halt_mask) != halt_mask) {
            printf_P(
                PSTR("breaking due to %S signal\n"),
                !(xflags & halt_mask & RESET) ? PSTR("reset") : PSTR("halt"));
            break;
        }
    }
    clk_stop();
    CLK_LO;
    while (!bus_request())
        ;
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
    watch_flag = 0;

    if (cycles > 0)
        cycles--; // 0 based counter makes comparison cheaper

    bus_release();
    for (;;) {
        last_rd = GET_RD;
        last_wr = GET_WR;
        CLK_HI;
        CLK_LO;
        if ((last_rd && !GET_RD) || (last_wr && !GET_WR)) { // falling edge
            bus_stat status = bus_status();
            if (watch_flag || (status.xflags & halt_mask) != halt_mask)
                break;
            if (!IORQ_STATUS) {
                status.data = iorq_dispatch();
                if (!RD_STATUS) {
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
                if (!RD_STATUS) {
                    if (INRANGE(watches, MEMRD, status.addr)) {
                        bus_log(status);
                        uart_flush();
                    }
                    if (!M1_STATUS) {
                        if (!ignore_m1) {
                            if (INRANGE(watches, OPFETCH, status.addr)) {
                                if (bus_request()) {
                                    disasm_mem(status.addr, status.addr);
                                } else {
                                    printf_P(PSTR("bus request timed out"));
                                }
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
    while (!bus_request())
        ;
}