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

#ifndef UTIL_H
#define UTIL_H

/**
 * @file util.h Miscellaneous utility functions
 */

#include <stdint.h>

const char *strlookup(const char *str, uint32_t index); /**< Lookup a string by index in a NULL-separated PROGMEM array */

#define CLKOFF 0
#define CLKDIV1 1
#define CLKDIV8 2
#define CLKDIV64 3
#define CLKDIV256 4
#define CLKDIV1024 5
#define CLKEXTFALL 6
#define CLKEXTRISE 7

#define TCNT_TO_US(tcnt, freq) (tcnt / (freq/1000000UL))
#define TCNT_TO_MS(tcnt, freq) (tcnt / (freq/1000UL))

void config_timer(uint8_t timer, uint8_t prescaler);
uint16_t get_tcnt(uint8_t timer);
void set_tcnt(uint8_t timer, uint16_t value);

extern uint8_t clibuf[256];
void save_cli(int argc, char *argv[]);

#endif
