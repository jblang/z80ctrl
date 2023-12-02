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
 * @file rtc_ds1306.c DS1306+ RTC functions
 */

#include <time.h>

#include "rtc.h"
#include "rtc_ds1306.h"
#include "spi.h"

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

void rtc_read_burst(uint8_t start, uint8_t end, uint8_t values[])
{
    rtc_begin();
    spi_exchange(start);
    for (uint8_t reg = start; reg <= end; reg++)
        *values++ = spi_exchange(0);
    rtc_end();
}

void rtc_write_burst(uint8_t start, uint8_t end, uint8_t values[])
{
    rtc_begin();
    spi_exchange(start | RTC_WRITE);
    for (uint8_t reg = start; reg <= end; reg++)
        spi_exchange(*values++);
    rtc_end();
}

uint8_t rtc_read(uint8_t reg)
{
    uint8_t value;
    rtc_read_burst(reg, reg, &value);
    return value;
}

void rtc_write(uint8_t reg, uint8_t value)
{
    rtc_write_burst(reg, reg, &value);
}

#define frombcd(value) (((value) >> 4) * 10 + ((value) & 0xF))

void rtc_get_date(struct tm* date)
{
    uint8_t data[7];

    rtc_read_burst(RTC_SEC, RTC_YEAR, data);
    date->tm_year = frombcd(data[RTC_YEAR]);
    date->tm_mon = frombcd(data[RTC_MONTH]);
    date->tm_mday = frombcd(data[RTC_DAY]);
    date->tm_wday = data[RTC_WEEKDAY];
    if ((data[RTC_HOUR] & (1 << RTC_1224))) {
        // 12-hour mode
        date->tm_hour = frombcd(data[RTC_HOUR] & 0x1F);
        if ((data[RTC_HOUR] & (1 << RTC_AMPM))) // PM
            date->tm_hour += 12;
    } else {
        // 24-hour mode
        date->tm_hour = frombcd(data[RTC_HOUR]);
    }
    date->tm_min = frombcd(data[RTC_MIN]);
    date->tm_sec = frombcd(data[RTC_SEC]);
}

#define tobcd(value) (((value / 10) << 4) | (value % 10))

void rtc_set_date(struct tm* date)
{
    uint8_t data[7];

    data[RTC_YEAR] = tobcd(date->tm_year);
    data[RTC_MONTH] = tobcd(date->tm_mon);
    data[RTC_DAY] = tobcd(date->tm_mday);
    data[RTC_WEEKDAY] = date->tm_wday;
    data[RTC_HOUR] = tobcd(date->tm_hour);
    data[RTC_MIN] = tobcd(date->tm_min);
    data[RTC_SEC] = tobcd(date->tm_sec);
    rtc_write_burst(RTC_SEC, RTC_YEAR, data);
}