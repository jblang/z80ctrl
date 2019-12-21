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
 * @file rtc.h DS1306 RTC functions
 */

#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <time.h>

#include "iox.h"

#define RTC_WRITE 0x80

#define RTC_SEC 0x0
#define RTC_MIN 0x1
#define RTC_HOUR 0x2
#define RTC_WEEKDAY 0x3
#define RTC_DAY 0x4
#define RTC_MONTH 0x5
#define RTC_YEAR 0x6
#define RTC_ALARM0 0x7
#define RTC_ALARM1 0xB
#define RTC_CONTROL 0xF
#define RTC_STATUS 0x10
#define RTC_CHARGE 0x11

#define RTC_RAM_BASE 0x20
#define RTC_RAM_SIZE 96

#define RTC_AIE0 0
#define RTC_AIE1 1
#define RTC_HZ 2
#define RTC_WP 6

#define RTC_IRQF0 0
#define RTC_IRQF1 1

#define RTC_1224 6
#define RTC_AMPM 5

#define RTC_SEL AUX1_SEL
#define RTC_DESEL AUX2_SEL

void rtc_read(uint8_t start, uint8_t end, uint8_t values[]);
void rtc_write(uint8_t start, uint8_t end, uint8_t values[]);

uint8_t rtc_read1(uint8_t reg);
void rtc_write1(uint8_t reg, uint8_t value);

void rtc_get_date(struct tm *date);
void rtc_set_date(struct tm *date);

#endif