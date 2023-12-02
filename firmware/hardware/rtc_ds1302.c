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
 * @file rtc_ds1302.c DS1302+ RTC functions
 */

#include <time.h>
#include <stdio.h>
#include <util/delay.h>

#include "rtc.h"
#include "rtc_ds1302.h"
#include "spi.h"

void rtc_begin()
{
    SPI_DISABLE;
    SCK_LO;
    _delay_us(4);
    RTC_SEL;
    _delay_us(4);
}

void rtc_end()
{
    RTC_DESEL;    
    SPI_ENABLE;
}

void rtc_byte_out(uint8_t value)
{
    MISO_OUTPUT;
    for (uint8_t i = 0; i < 8; i++) {
        if (value & 1)
            MISO_HI;
        else
            MISO_LO;
        SCK_LO;
        _delay_us(2);
        SCK_HI;
        _delay_us(2);
        value >>= 1;
    }
    MISO_INPUT;
}

uint8_t rtc_byte_in()
{
    uint8_t value = 0;
    MISO_INPUT;
    for (uint8_t i = 0; i < 8; i++) {
        value >>= 1;
        SCK_HI;
        _delay_us(2);
        SCK_LO;
        _delay_us(2);
        if (GET_MISO)
            value |= 0x80;
    }
    return value;
}

void rtc_read_burst(uint8_t addr, uint8_t len, uint8_t *values)
{
    rtc_begin();
    rtc_byte_out(addr | RTC_READ);
    for (uint8_t i = 0; i < len; i++)
        *values++ = rtc_byte_in();
    rtc_end();
}

void rtc_write_burst(uint8_t addr, uint8_t len, uint8_t *values)
{
    rtc_begin();
    rtc_byte_out(addr & ~RTC_READ);
    for (uint8_t i = 0; i <= len; i++)
        rtc_byte_out(*values++);
    rtc_end();
}

uint8_t rtc_read(uint8_t reg)
{
    uint8_t value;
    rtc_begin();
    rtc_byte_out(reg | RTC_READ);
    value = rtc_byte_in();
    rtc_end();
    return value;
}

void rtc_write(uint8_t reg, uint8_t value)
{
    rtc_begin();
    rtc_byte_out(reg & ~RTC_READ);
    rtc_byte_out(value);
    rtc_end();
}

#define frombcd(value) (((value) >> 4) * 10 + ((value) & 0xF))

void rtc_get_date(struct tm *date)
{
    uint8_t data[8];

    rtc_read_burst(RTC_CLKBURST, RTC_CLKSIZE, data);
    date->tm_year = frombcd(data[6]);
    date->tm_mon = frombcd(data[4]);
    date->tm_mday = frombcd(data[3]);
    date->tm_wday = data[5];
    if ((data[2] & (1 << RTC_1224))) {
        // 12-hour mode
        date->tm_hour = frombcd(data[2] & 0x1F);
        if ((data[2] & (1 << RTC_AMPM))) // PM
            date->tm_hour += 12;
    } else {
        // 24-hour mode
        date->tm_hour = frombcd(data[2]);
    }
    date->tm_min = frombcd(data[1]);
    date->tm_sec = frombcd(data[0] & 0x7f);
}

#define tobcd(value) (((value / 10) << 4) | (value % 10))

void rtc_set_date(struct tm *date)
{
    uint8_t data[8];

    data[6] = tobcd(date->tm_year);
    data[5] = date->tm_wday;
    data[4] = tobcd(date->tm_mon);
    data[3] = tobcd(date->tm_mday);
    data[2] = tobcd(date->tm_hour);
    data[1] = tobcd(date->tm_min);
    data[0] = tobcd(date->tm_sec);
    rtc_write(RTC_WPREG, 0);
    rtc_write_burst(RTC_CLKBURST, RTC_CLKSIZE, data);
}