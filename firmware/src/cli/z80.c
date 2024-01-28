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
 * @file z80.c Z80-related commands
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/bus.h"
#include "hardware/z80.h"

#include "util/disasm.h"
#include "util/pgmspace.h"
#include "util/string.h"

/**
 * Disassemble code from memory
 */
void cli_disasm(int argc, char* argv[])
{
    uint32_t start, end;
    if (argc < 2) {
        printf_P(PSTR("usage: disasm <start> [end]\n"));
        return;
    }
    start = strtoul(argv[1], NULL, 16) & 0xffff;
    if (argc < 3)
        end = start + 0xf;
    else
        end = strtoul(argv[2], NULL, 16) & 0xffff;
    disasm_mem(start, end);
}

/**
 * Reduce the Z80 clock speed by the specified factor
 */
void cli_clkdiv(int argc, char* argv[])
{
    uint8_t tmp = 0;
    if (argc >= 2)
        tmp = strtoul(argv[1], NULL, 10);
    if (tmp > 1)
        set_clkdiv(tmp);
    else
        printf_P(PSTR("usage: clkdiv <divider>; minimum divider is 2\n"));
    uint16_t freq = F_CPU / get_clkdiv() / 1000;
    printf_P(PSTR("current speed is %u.%03u MHz (clkdiv=%d)\n"), freq / 1000, freq - (freq / 1000) * 1000, get_clkdiv());
}

/**
 * Reset the processor and optionally set up a reset vector
 */
void cli_reset(int argc, char* argv[])
{
    uint32_t addr = 0;
    if (argc >= 2) {
        addr = strtoul(argv[1], NULL, 16) & 0xfffff;
    }
    z80_reset(addr);
}

/**
 * Run the processor at full speed from an optional starting address
 */
void cli_run(int argc, char* argv[])
{
    if (argc >= 2) {
        uint32_t addr = strtoul(argv[1], NULL, 16) & 0xfffff;
        z80_reset(addr);
    }
    z80_run();
}

/**
 * Run the processor with debugging enabled from an optional starting address
 */
void cli_debug(int argc, char* argv[])
{
    if (argc >= 2) {
        uint32_t addr = strtoul(argv[1], NULL, 16) & 0xfffff;
        z80_reset(addr);
    }
    z80_debug(1);
}

/**
 * Run the processor for a specified number of instructions
 */
void cli_step(int argc, char* argv[])
{
    uint32_t cycles = 1;
    if (argc >= 2)
        cycles = strtoul(argv[1], NULL, 10);
    z80_debug(cycles);
}

/**
 * Display the bus status
 */
void cli_bus(int argc, char* argv[])
{
    bus_log(bus_status());
}

/**
 * Enable or disable halt
 */
void cli_halt(int argc, char* argv[])
{
    if (argc == 2) {
        if (strcmp_P(argv[1], PSTR("on")) == 0 || strcmp_P(argv[1], PSTR("both")) == 0)
            set_haltmask(RESET | HALT);
        else if (strcmp_P(argv[1], PSTR("off")) == 0 || strcmp_P(argv[1], PSTR("reset")) == 0)
            set_haltmask(RESET);
        else if (strcmp_P(argv[1], PSTR("halt")) == 0)
            set_haltmask(HALT);
        else if (strcmp_P(argv[1], PSTR("none")) == 0)
            set_haltmask(0);
    }
    printf_P(PSTR("halt on reset signal is %S\n"), get_haltmask() & RESET ? PSTR("enabled") : PSTR("disabled"));
    printf_P(PSTR("halt on halt signal is %S\n"), get_haltmask() & HALT ? PSTR("enabled") : PSTR("disabled"));
}

/**
 * Interrogate or set breakpoint and watch ranges
 */
void cli_breakwatch(int argc, char* argv[])
{
    range* ranges;
    uint8_t type;

    if (strcmp_P(argv[0], PSTR("break")) == 0)
        ranges = breaks;
    else
        ranges = watches;

    // If no parameters given, show current status
    if (argc == 1) {
        printf_P(PSTR("%s status:\n"), argv[0]);
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            if (!ENABLED(ranges, i))
                printf_P(PSTR("\t%S\tdisabled\n"), skip_strings(debug_names, i));
            else
                printf_P(PSTR("\t%S\t%04x-%04x\n"), skip_strings(debug_names, i), ranges[i].start, ranges[i].end);
        }
        printf_P(PSTR("\nusage:\n\t%s <type> [start] [end]\n"), argv[0]);
        printf_P(PSTR("\t%s <type> off to disable type\n"), argv[0]);
        printf_P(PSTR("\t%s off to disable all\n"), argv[0]);
        return;
    }
    if (strcmp_P(argv[1], PSTR("off")) == 0) {
        // turn off all ranges
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            ranges[i].start = 0xffff;
            ranges[i].end = 0;
        }
        return;
    } else {
        // find the debugging type that the user specified
        for (type = 0; type < DEBUGCNT; type++)
            if (strcmp_P(argv[1], skip_strings(debug_names, type)) == 0)
                break;
        if (type == DEBUGCNT) {
            printf_P(PSTR("error: unknown type\n"));
            return;
        }
        if (argc == 2) {
            // no range specified, enable for 0x0000-0xffff
            ranges[type].start = 0;
            ranges[type].end = 0xffff;
        } else if (argc >= 3) {
            if (strcmp_P(argv[2], PSTR("off")) == 0) {
                ranges[type].start = 0xffff;
                ranges[type].end = 0;
            } else {
                // get starting address
                ranges[type].start = strtoul(argv[2], NULL, 16);
                if (argc >= 4)
                    // get ending address if specified
                    ranges[type].end = strtoul(argv[3], NULL, 16);
                else
                    // if no ending address, start and end are the same
                    ranges[type].end = ranges[type].start;
            }
        }
    }
}