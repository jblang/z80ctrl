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

#ifndef IHEX_H
#define IHEX_H

#include <stdint.h>

enum ihex_rectype {
    IHEX_DATA = 0,
    IHEX_EOF,
    IHEX_ESA,
    IHEX_SSA,
    IHEX_ELA,
    IHEX_SLA
};

enum ihex_rc {
    IHEX_OK = 0,
    IHEX_FORMAT, 
    IHEX_COUNT,
    IHEX_CKSUM,
    IHEX_RECTYPE
};

typedef struct _ihex_res {
    uint8_t count;
    uint16_t addr;
    uint8_t type;
    uint8_t rc;
} ihex_res;

ihex_res ihex_to_bin(char *ihex, uint8_t *bin);
void bin_to_ihex(uint8_t *bin, char *ihex, uint16_t addr, uint8_t count);


#endif