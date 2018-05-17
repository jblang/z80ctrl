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
#include <string.h>
#include "ihex.h"

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

uint8_t tohex(uint8_t nyb)
{
    if (0 <= nyb && nyb <= 9)
        return '0' + nyb;
    else if (0xa <= nyb && nyb <= 0xf)
        return 'A' + nyb - 10;
    else
        return 0;
}

ihex_res ihex_to_bin(char *ihex, uint8_t *bin) 
{
    ihex_res res;
    int i;

    if (strlen(ihex) < 11) {
        res.rc == IHEX_FORMAT;
        return res;
    }
    if (ihex[0] != ':') {
        res.rc = IHEX_FORMAT;
        return res;
    }
    for (i = 1; i < strlen(ihex); i++) {
        if (fromhex(ihex[i]) > 0xf && ihex[i] != '\r' && ihex[i] != '\n') {
            res.rc = IHEX_FORMAT;
            return res;
        }
    }
    res.count = fromhex(ihex[1]) << 4 | fromhex(ihex[2]);
    if (strlen(ihex) < 11 + res.count) {
        res.rc = IHEX_COUNT;
        return res;
    }
    res.addr = fromhex(ihex[3]) << 12 | fromhex(ihex[4]) << 8 | fromhex(ihex[5]) << 4 | fromhex(ihex[6]);
    res.type = fromhex(ihex[7]) << 4 | fromhex(ihex[8]);
    uint8_t check1 = res.count + (res.addr >> 8) + (res.addr & 0xff) + res.type;
    for (i = 0; i < res.count; i++) {
        bin[i] = fromhex(ihex[i*2+9]) << 4 | fromhex(ihex[i*2+10]);
        check1 += bin[i];
    }
    check1 = ~check1 + 1;
    uint8_t check2 = fromhex(ihex[res.count*2+9]) << 4 | fromhex(ihex[res.count*2+10]);
    if (check1 != check2) {
        res.rc = IHEX_CKSUM;
        return res;
    }
    if (res.type == IHEX_DATA || res.type == IHEX_EOF)
        res.rc = IHEX_OK;
    else
        res.rc = IHEX_RECTYPE;
    return res;
}

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