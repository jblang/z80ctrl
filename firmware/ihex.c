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
 * @file ihex.c Functions for loading and saving Intel HEX files
 */

#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "ihex.h"
#include "bus.h"
#include "util.h"

/**
 * Record types for Intel HEX files
 */
enum ihex_rectype {
    IHEX_DATA = 0,
    IHEX_EOF,
    IHEX_ESA,
    IHEX_SSA,
    IHEX_ELA,
    IHEX_SLA
};

/**
 * Result codes from parsing a single Intel HEX record
 */
enum ihex_rc {
    IHEX_OK = 0,
    IHEX_FORMAT, 
    IHEX_COUNT,
    IHEX_CKSUM,
    IHEX_RECTYPE
};

/**
 * Error text corresponding to result codes
 */
const char ihex_rc_text[] PROGMEM = {
    "ok\0"
    "invalid record format\0"
    "insufficient bytes\0"
    "checksum mismatch\0"
    "unsupported record type"
};

/**
 * Decoded Intel HEX record
 */
typedef struct {
    uint8_t count;
    uint16_t addr;
    uint8_t type;
    uint8_t rc;
} ihex_rec;

/**
 * Convert from a hex digit to a binary nybble
 */
uint8_t fromhex(char hex)
{
    if ('0' <= hex && hex <= '9')
        return hex - '0';
    else if ('A' <= hex && hex <= 'F')
        return hex - 'A' + 10;
    else if ('a' <= hex && hex <= 'f')
        return hex - 'a' + 10;
    else
        return 255;
}

/**
 * Convert a binary nybble to a hex digit
 */
uint8_t tohex(uint8_t nyb)
{
    if (0 <= nyb && nyb <= 9)
        return '0' + nyb;
    else if (0xa <= nyb && nyb <= 0xf)
        return 'A' + nyb - 10;
    else
        return 0;
}

/**
 * Parse a single Intel HEX record and convert it to binary
 */
ihex_rec ihex_to_bin(char *ihex, uint8_t *bin) 
{
    ihex_rec record;
    int i;

    if (strlen(ihex) < 11) {
        record.rc = IHEX_FORMAT;
        return record;
    }
    if (ihex[0] != ':') {
        record.rc = IHEX_FORMAT;
        return record;
    }
    for (i = 1; i < strlen(ihex); i++) {
        if (fromhex(ihex[i]) > 0xf && ihex[i] != '\r' && ihex[i] != '\n') {
            record.rc = IHEX_FORMAT;
            return record;
        }
    }
    record.count = fromhex(ihex[1]) << 4 | fromhex(ihex[2]);
    if (strlen(ihex) < 11 + record.count) {
        record.rc = IHEX_COUNT;
        return record;
    }
    record.addr = fromhex(ihex[3]) << 12 | fromhex(ihex[4]) << 8 | fromhex(ihex[5]) << 4 | fromhex(ihex[6]);
    record.type = fromhex(ihex[7]) << 4 | fromhex(ihex[8]);
    uint8_t check1 = record.count + (record.addr >> 8) + (record.addr & 0xff) + record.type;
    for (i = 0; i < record.count; i++) {
        bin[i] = fromhex(ihex[i*2+9]) << 4 | fromhex(ihex[i*2+10]);
        check1 += bin[i];
    }
    check1 = ~check1 + 1;
    uint8_t check2 = fromhex(ihex[record.count*2+9]) << 4 | fromhex(ihex[record.count*2+10]);
    if (check1 != check2) {
        record.rc = IHEX_CKSUM;
        return record;
    }
    if (record.type == IHEX_DATA || record.type == IHEX_EOF)
        record.rc = IHEX_OK;
    else
        record.rc = IHEX_RECTYPE;
    return record;
}

/**
 * Load an Intel HEX file into memory
 */
ihex_res load_ihex(FILE *file)
{
    char ihex[524];
    uint8_t bin[256];
    uint16_t line = 0;
    ihex_rec record;
    ihex_res result;
    result.min = 0xffff;
    result.max = 0;
    result.total = 0;
    result.errors = 0;
    for (;;) {
        if (fgets(ihex, 524, file) == NULL)
            break;
        if (strlen(ihex) == 0)
            break;
        line++;
        record = ihex_to_bin(ihex, bin);
        if (record.rc == IHEX_OK && record.type == IHEX_DATA && record.count > 0) {
            mem_write_banked(record.addr, bin, record.count);
            result.total += record.count;
            if (record.addr < result.min)
                result.min = record.addr;
            if (record.addr + record.count - 1 > result.max)
                result.max = record.addr + record.count - 1;
        } else if (record.rc == IHEX_OK && record.count == 0) {
            break;
        } else {
            printf_P(PSTR("error: %S on line %d\n"), strlookup(ihex_rc_text, record.rc), line);
            result.errors++;
        }
    }
    return result;
}

#define BYTESPERLINE 16

/**
 * Encode a byte array to a single Intel HEX record
 */
void bin_to_ihex(uint8_t *bin, char *ihex, uint16_t addr, uint8_t count) 
{
    uint8_t check = count + (addr >> 8) + (addr & 0xff);
    int i;

    ihex[0] = ':';
    ihex[1] = tohex(count >> 4);
    ihex[2] = tohex(count & 0xf);
    ihex[3] = tohex((addr >> 12) & 0xf);
    ihex[4] = tohex((addr >> 8) & 0xf);
    ihex[5] = tohex((addr >> 4) & 0xf);
    ihex[6] = tohex(addr & 0xf);
    ihex[7] = '0';
    if (count > 0) {
        ihex[8] = '0';
    } else {
        check += 1;
        ihex[8] = '1';
    }
    for (i = 0; i < count; i++) {
        ihex[i*2+9] = tohex(bin[i] >> 4);
        ihex[i*2+10] = tohex(bin[i] & 0xf);
        check += bin[i];
    }
    check = ~check + 1;
    ihex[count*2+9] = tohex(check >> 4);
    ihex[count*2+10] = tohex(check & 0xf);
    ihex[count*2+11] = '\0';
}

/**
 * Save a range of memory to an Intel HEX file
 */
int save_ihex(uint32_t start, uint16_t end, FILE *file)
{
    uint8_t count;
    char bin[BYTESPERLINE];
    char ihex[BYTESPERLINE*2+12];
    uint16_t i;
    for (;;) {
        if (end - start + 1 > BYTESPERLINE)
            count = BYTESPERLINE;
        else
            count = end - start + 1;
        mem_read_banked(start, bin, count);
        bin_to_ihex(bin, ihex, start, count);
        if (fprintf_P(file, PSTR("%s\n"), ihex) == EOF)
            return EOF;
        start += count;
        if (start > end) {
            bin_to_ihex(bin, ihex, 0, 0);
            if (fprintf_P(file, PSTR("%s\n"), ihex) == EOF)
                return EOF;
            break;
        }
    }
    return 0;
}