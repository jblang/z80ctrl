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
 * @file z80.h Z80 debugger
 */

#ifndef Z80_H
#define Z80_H

#include <stdint.h>
#include "bus.h"


enum {MEMRD, MEMWR, IORD, IOWR, OPFETCH, BUS, DEBUGCNT};

typedef struct {
        uint16_t start;
        uint16_t end;
} range;

extern range breaks[];
extern range watches[];
extern const char debug_names[];

#define INRANGE(ranges, type, addr) ((ranges)[(type)].start <= (addr) && (addr) <= (ranges)[type].end)
#define ENABLED(ranges, type) ((ranges)[(type)].start <= (ranges)[(type)].end)

extern uint8_t z80_uart[];

void z80_reset(uint16_t addr);
void z80_run(void);
void z80_debug(uint32_t cycles);
void z80_buslog(bus_stat status);

#endif