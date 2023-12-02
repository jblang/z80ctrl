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
 * @file hardware.c commands for interacting with hardware
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hardware/mcp23s17.h"
#include "hardware/rtc.h"
#include "hardware/uart.h"

#include "util/pgmspace.h"

/**
 * Set the baud rate for the UART
 */
void cli_baud(int argc, char *argv[]) {
    if (argc != 3) {
        printf_P(PSTR("usage: baud <uart> <baud>\n"));
        return;
    }
    uint8_t uart = strtoul(argv[1], NULL, 10) & 1;
    uint32_t requested = strtoul(argv[2], NULL, 10);
    uint16_t ubrr = 0;
    uint32_t actual = 0;
    // Find closest possible actual baud rate
    for (;;) {
        actual = (uint32_t)F_CPU / ((uint32_t)16 * ((uint32_t)ubrr + (uint32_t)1));
        if (actual <= requested)
            break;
        ubrr++;
        // stop if we have maxed out UBRR
        if (ubrr == 0)
            break;
    }
    printf_P(PSTR("UART %u: requested: %lu, actual: %lu\n"), uart, requested, actual);
    uart_flush();
    uart_init(uart, ubrr);
}

#ifdef USE_RTC
/**
 * Display or set the date on the RTC
 */
void cli_date(int argc, char *argv[])
{
    struct tm date;
    char datestr[30];
    if (argc < 8) {
        rtc_get_date(&date);
        printf_P(PSTR("The time is %d:%02d:%02d on %d/%d/%02d\n"), date.tm_hour, date.tm_min, date.tm_sec, date.tm_mon, date.tm_mday, date.tm_year);
        printf_P(PSTR("usage: date <yy mm dd wd hh mm ss>\n"));
        return;
    }

    date.tm_year = strtoul(argv[1], NULL, 10);
    date.tm_mon = strtoul(argv[2], NULL, 10);
    date.tm_mday = strtoul(argv[3], NULL, 10);
    date.tm_wday = strtoul(argv[4], NULL, 10);
    date.tm_hour = strtoul(argv[5], NULL, 10);
    date.tm_min = strtoul(argv[6], NULL, 10);
    date.tm_sec = strtoul(argv[7], NULL, 10);
    rtc_set_date(&date);
}
#endif

void cli_ioxread(int argc, char *argv[])
{
    if (argc != 3){ 
        printf_P(PSTR("usage: ioxread <addr> <reg>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0x7;
    if (addr < 1 || addr > 7) {
        printf_P(PSTR("invalid address. must be between 1 and 7\n"));
        return;
    }
    uint8_t reg = strtoul(argv[2], NULL, 16) & 0x1F;
    uint8_t value = iox_read(addr, reg);
    printf_P(PSTR("Read %02x from io expander %x register %02x\n"), value, addr, reg);
}

void cli_ioxwrite(int argc, char *argv[])
{
    if (argc != 4){ 
        printf_P(PSTR("usage: ioxwrite <addr> <reg> <value>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0x7;
    if (addr < 1 || addr > 7) {
        printf_P(PSTR("invalid address. must be between 1 and 7\n"));
        return;
    }
    uint8_t reg = strtoul(argv[2], NULL, 16) & 0x1F;
    uint8_t value = strtoul(argv[3], NULL, 16);
    iox_write(addr, reg, value);
}
