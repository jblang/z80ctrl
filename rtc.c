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
 * @file rtc.c DS1306+ RTC functions
 */

#include "rtc.h"
#include "spi.h"

static uint8_t rtc_def_regaddr = 0;

void rtc_begin()
{
    SPI_SLOW;
    SPI_PHASE1;
    RTC_SEL;
}

void rtc_end()
{
    RTC_DESEL;
    SPI_PHASE0;
    SPI_FAST;
}

void rtc_read(uint8_t start, uint8_t end, uint8_t values[])
{
    rtc_begin();
    spi_exchange(start);
    for (uint8_t reg = start; reg <= end; reg++)
        values[reg - start] = spi_exchange(0);
    rtc_end();
}

void rtc_write(uint8_t start, uint8_t end, uint8_t values[])
{
    rtc_begin();
    spi_exchange(start | RTC_WRITE);
    for (uint8_t reg = start; reg <= end; reg++)
        spi_exchange(values[reg - start]);
    rtc_end();
}

uint8_t rtc_read1(uint8_t reg)
{
    uint8_t value;
    rtc_read(reg, reg, &value);
    return value;
}

void rtc_write1(uint8_t reg, uint8_t value)
{
    rtc_write(reg, reg, &value);
}

rtc_date_t rtc_get_date()
{
    uint8_t data[7];
    rtc_date_t date;

    rtc_read(RTC_SEC, RTC_YEAR, data);
    date.year = (data[RTC_YEAR] >> 4) * 10 + (data[RTC_YEAR] & 0xF);
    date.month = (data[RTC_MONTH] >> 4) * 10 + (data[RTC_MONTH] & 0xF);
    date.day = (data[RTC_DAY] >> 4) * 10 + (data[RTC_DAY] & 0xF);
    date.weekday = data[RTC_WEEKDAY];
    if ((data[RTC_HOUR] & (1 << RTC_1224))) {
        // 12-hour mode
        date.hour = ((data[RTC_HOUR] >> 4) & 1) * 10 + (data[RTC_HOUR] & 0xF);
        if ((data[RTC_HOUR] & (1 << RTC_AMPM))) // PM
            date.hour += 12;
    } else {
        // 24-hour mode
        date.hour = ((data[RTC_HOUR] >> 4) & 3) * 10 + (data[RTC_HOUR] & 0xF);
    }
    date.min = (data[RTC_MIN] >> 4) * 10 + (data[RTC_MIN] & 0xF);
    date.sec = (data[RTC_SEC] >> 4) * 10 + (data[RTC_SEC] & 0xF);
    return date;
}

void rtc_set_date(rtc_date_t date)
{
    uint8_t data[7];

    data[RTC_YEAR] = ((date.year / 10) << 4) | (date.year % 10);
    data[RTC_MONTH] = ((date.month / 10) << 4) | (date.month % 10);
    data[RTC_DAY] = ((date.day / 10) << 4) | (date.day % 10);
    data[RTC_WEEKDAY] = date.weekday;
    data[RTC_HOUR] = ((date.hour / 10) << 4) | (date.hour % 10);
    data[RTC_MIN] = ((date.min / 10) << 4) | (date.min % 10);
    data[RTC_SEC] = ((date.sec / 10) << 4) | (date.sec % 10);
    rtc_write(RTC_SEC, RTC_YEAR, data);
}

void rtc_defreg(uint8_t regaddr)
{
    rtc_def_regaddr = regaddr;
}

void rtc_writedef(uint8_t data)
{
    rtc_write1(rtc_def_regaddr, data);
}

uint8_t rtc_readdef()
{
    return rtc_read1(rtc_def_regaddr);
}