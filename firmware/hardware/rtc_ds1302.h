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
 * @file rtc_ds1302.h DS1302+ RTC functions
 */

#ifndef HARDWARE_RTC_DS1302_H
#define HARDWARE_RTC_DS1302_H

#include <stdint.h>
#include <time.h>

#include "mcp23s17.h"

#define RTC_READ 1

#define RTC_SEC 0x80
#define RTC_MIN 0x82
#define RTC_HOUR 0x84
#define RTC_DAY 0x86
#define RTC_MONTH 0x88
#define RTC_WEEKDAY 0x8A
#define RTC_YEAR 0x8C
#define RTC_WPREG 0x8E
#define RTC_CHARGE 0x90
#define RTC_CLKBURST 0xBE
#define RTC_CLKSIZE 8

#define RTC_RAMBASE 0xC0
#define RTC_RAMSIZE 31
#define RTC_RAMBURST 0xFE

#define RTC_CHBIT 7
#define RTC_WPBIT 7
#define RTC_1224 7
#define RTC_AMPM 5

#define RTC_SEL AUX1_SEL
#define RTC_DESEL AUX2_SEL

#endif