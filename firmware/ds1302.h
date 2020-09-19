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
 * @file ds1302.h DS1302+ RTC functions
 */

#ifndef DS1302_H
#define DS1302_H

#include <stdint.h>
#include <time.h>

#include "iox.h"

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

void rtc_read_burst(uint8_t addr, uint8_t len, uint8_t values[]);
void rtc_write_burst(uint8_t addr, uint8_t len, uint8_t values[]);

uint8_t rtc_read(uint8_t reg);
void rtc_write(uint8_t reg, uint8_t value);

void rtc_get_date(struct tm *date);
void rtc_set_date(struct tm *date);

#endif