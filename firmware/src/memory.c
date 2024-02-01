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
 * @file memory.c memory-related commands
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "memory.h"
#include "tms9918.h"

#include "harvard.h"

#include "z80ctrl.h"

#if defined(BANK_PORT) || defined(BANK_BASE)
/**
 * Set the base address in memory
 */
void cli_base(int argc, char* argv[])
{
    if (argc == 2) {
        if (base_addr & 0xFFFF != 0)
            printf_P(PSTR("base address must be a multiple of 0x10000"));
        else
            base_addr = strtoul(argv[1], NULL, 16) & 0xF0000;
    } else {
        printf_P(PSTR("current base address is %05lX\n"), base_addr);
    }
}
#endif

/**
 * Generate a hex dump from memory
 */
void cli_dump(int argc, char* argv[])
{
    uint8_t tms = (strcmp_P(argv[0], PSTR("tmsdump")) == 0);
    if (argc < 2) {
        printf_P(PSTR("usage: %s [start] [end]\n"), argv[0]);
    }

    static uint32_t start = 0;
    uint32_t end = 0;
    if (argc >= 2)
        start = strtoul(argv[1], NULL, 16);
    if (argc >= 3)
        end = strtoul(argv[2], NULL, 16);

    uint8_t buf[16];

    uint8_t buflen = 16;
    if (screenwidth < 80)
        buflen = 8;

    if (end == 0)
        end = start + (screenheight - 3) * buflen;

    printf_P(PSTR("ADDR   "));
    for (uint8_t j = 0; j < buflen; j++) {
        printf("+%X ", j);
        if (j % 4 == 3 && screenwidth > 41)
            printf_P(PSTR(" "));
    }
    if (screenwidth > 40)
        printf_P(PSTR(" "));
    for (uint8_t j = 0; j < buflen; j++)
        printf_P(PSTR("%X"), j);
    printf_P(PSTR("\n"));

    for (uint32_t i = start; i <= end;) {
        printf_P(PSTR("%05lX  "), base_addr + i);
#ifdef TMS_BASE
        if (tms)
            tms_read(i, buf, buflen);
        else
#endif
            mem_read_banked(i, buf, buflen);
        for (uint8_t j = 0; j < buflen; j++) {
            printf_P(PSTR("%02X "), buf[j]);
            if (j % 4 == 3 && screenwidth > 41)
                printf_P(PSTR(" "));
        }
        if (screenwidth > 40)
            printf_P(PSTR(" "));
        for (uint8_t j = 0; j < buflen; j++, i++) {
            if (0x20 <= buf[j] && buf[j] <= 0x7e)
                printf_P(PSTR("%c"), buf[j]);
            else
                printf_P(PSTR("."));
        }
        printf_P(PSTR("\n"));
    }
    start = end + buflen;
}

/**
 * Fill memory with a specified byte for a specified range
 */
void cli_fill(int argc, char* argv[])
{
    if (argc != 4) {
        printf_P(PSTR("usage: fill <start> <end> <value>\n"));
        return;
    }
    uint8_t tms = (strcmp_P(argv[0], PSTR("tmsfill")) == 0);
    uint32_t start = strtoul(argv[1], NULL, 16) & 0xfffff;
    uint32_t end = strtoul(argv[2], NULL, 16) & 0xfffff;
    uint8_t buf[256];
    if (strcmp_P(argv[3], PSTR("asc")) == 0) {
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = i;
    } else if (strcmp_P(argv[3], PSTR("desc")) == 0) {
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = 255 - i;
    } else {
        uint8_t value = strtoul(argv[3], NULL, 16) & 0xff;
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = value;
    }

    for (;;) {
        if (end - start > 256) {
#ifdef TMS_BASE
            if (tms)
                tms_write(start, buf, 256);
            else
#endif
                mem_write_banked(start, buf, 256);
            start += 256;
        } else {
#ifdef TMS_BASE
            if (tms)
                tms_write(start, buf, end - start + 1);
            else
#endif
                mem_write_banked(start, buf, end - start + 1);
            break;
        }
    }
}

/**
 * Poke values into memory
 */
void cli_poke(int argc, char* argv[])
{
    uint8_t value;
    if (argc < 2)
        printf_P(PSTR("usage: poke <start> [value...]\n"));
    uint32_t addr = strtoul(argv[1], NULL, 16);
    if (argc >= 3) {
        for (uint8_t i = 2; i < argc; i++) {
            value = strtoul(argv[i], NULL, 16) & 0xff;
            mem_write_banked(addr, &value, 1);
            addr++;
        }
        return;
    }
    printf_P(PSTR("enter valid hex to replace; blank to leave unchanged; 'x' to exit.\n"));
    for (;;) {
        char buf[16];
        mem_read_banked(addr, &value, 1);
        printf_P(PSTR("%05lX=%02X : "), addr + base_addr, value);
        if (fgets(buf, sizeof buf - 1, stdin) == NULL) {
            break;
        } else {
            char* end;
            value = strtoul(buf, &end, 16);
            if (*buf == '\n' || *buf == '\r' || *buf == '\0') {
                addr++;
                continue;
            } else if (end == buf) {
                break;
            } else {
                mem_write_banked(addr, &value, 1);
                addr++;
            }
        }
    }
}

#ifdef SST_FLASH
/**
 * Erase a flash sector or entire chip
 */
void cli_erase(int argc, char* argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: erase <addr> | erase all\n"));
        return;
    }
    uint32_t addr;
    if (strcmp_P(argv[1], PSTR("all")) == 0)
        addr = 0x80000;
    else
        addr = strtoul(argv[1], NULL, 16);
    sst_erase(addr);
}
#endif