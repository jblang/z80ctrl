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
 * @file tms9918.c TMS9918-related functions
 */
#ifdef TMS_BASE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "hardware/io.h"

#include "util/pgmspace.h"

#include "hardware/font.h"
#include "hardware/tms9918.h"

uint8_t tms_base;
uint16_t control_bits;
uint16_t name_table;
uint16_t color_table;
uint16_t pattern_table;
uint16_t sprite_attribute_table;
uint16_t sprite_pattern_table;
uint8_t screen_colors;
uint8_t pixel_colors;
uint8_t last_status;
uint8_t last_reg_write;
uint8_t reg_high_byte;

enum {
    // TMS9918A registers
    TMS_CONTROL0 = 0,
    TMS_CONTROL1,
    TMS_NAME_TABLE,
    TMS_COLOR_TABLE,
    TMS_PATTERN_TABLE,
    TMS_SPRITE_ATTRIBUTE_TABLE,
    TMS_SPRITE_PATTERN_TABLE,
    TMS_SCREEN_COLORS,

    // TMS9918A control register bits
    TMS_M3 = 2,
    TMS_EXT_VIDEO = 1,
    TMS_16K = 0x8000,
    TMS_DISPLAY_ENABLE = 0x4000,
    TMS_INT_ENABLE = 0x2000,
    TMS_M1 = 0x1000,
    TMS_M2 = 0x800,
    TMS_SPRITE_32X32 = 0x200,
    TMS_SPRITE_MAG = 0x100
};

void _tms_write(uint16_t addr, const void* buf, uint16_t len, uint8_t pgmspace)
{
    const uint8_t* bytebuf = buf;
    addr &= 0x3fff;
    addr |= 0x4000;
    DATA_OUTPUT;
    io_out(tms_base + 1, addr & 0xff);
    _delay_us(2);
    io_out(tms_base + 1, addr >> 8);
    _delay_us(2);
    for (uint16_t i = 0; i < len; i++) {
        if (pgmspace)
            io_out(tms_base, pgm_read_byte(&bytebuf[i]));
        else
            io_out(tms_base, bytebuf[i]);
        if (control_bits & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
}

void tms_write(uint32_t addr, const void* buf, uint32_t len)
{
    _tms_write(addr, buf, len, 0);
}

void tms_write_P(uint32_t addr, const void* buf, uint32_t len)
{
    _tms_write(addr, buf, len, 1);
}

void tms_read(uint32_t addr, void* buf, uint32_t len)
{
    uint8_t* bytebuf = buf;
    DATA_OUTPUT;
    addr &= 0x3fff;
    io_out(tms_base + 1, addr & 0xff);
    _delay_us(2);
    io_out(tms_base + 1, addr >> 8);
    _delay_us(2);
    DATA_INPUT;
    for (uint16_t i = 0; i < len; i++) {
        bytebuf[i] = io_in(tms_base);
        if (control_bits & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
}

uint8_t tms_readreg()
{
    return io_in(tms_base + 1);
}

void tms_writereg(uint8_t reg, uint8_t data)
{
    reg &= 0x07;
    reg |= 0x80;
    io_out(tms_base + 1, data);
    _delay_us(2);
    io_out(tms_base + 1, reg);
    _delay_us(2);
}

void tms_config()
{
    // Mask off unused bits in table addresses
    name_table &= 0x3c00;
    if (control_bits & TMS_M3) {
        color_table &= 0x2000;
        pattern_table &= 0x2000;
    } else {
        color_table &= 0x3fc0;
        pattern_table &= 0x3800;
    }
    sprite_attribute_table &= 0x3f80;
    sprite_pattern_table &= 0x3800;

    // Disable interrupts to prevent crash if last program had enabled them
    control_bits &= ~TMS_INT_ENABLE;

    // Write registers for current configuration
    tms_writereg(TMS_CONTROL0, control_bits & 0xff);
    tms_writereg(TMS_CONTROL1, control_bits >> 8);
    tms_writereg(TMS_NAME_TABLE, name_table / 0x400);
    if (control_bits & TMS_M3) {
        tms_writereg(TMS_COLOR_TABLE, color_table / 0x40 + 0x7f);
        tms_writereg(TMS_PATTERN_TABLE, pattern_table / 0x800 + 3);
    } else {
        tms_writereg(TMS_COLOR_TABLE, color_table / 0x40);
        tms_writereg(TMS_PATTERN_TABLE, pattern_table / 0x800);
    }
    tms_writereg(TMS_SPRITE_ATTRIBUTE_TABLE, sprite_attribute_table / 0x80);
    tms_writereg(TMS_SPRITE_PATTERN_TABLE, sprite_pattern_table / 0x800);
    tms_writereg(TMS_SCREEN_COLORS, screen_colors);
}

void tms_save_reg(uint8_t data)
{
    if (reg_high_byte) {
        if (data & 0x80) {
            switch (data & 0x7) {
            case TMS_CONTROL0:
                control_bits &= 0xff00;
                control_bits |= last_reg_write;
                break;
            case TMS_CONTROL1:
                control_bits &= 0xff;
                control_bits |= (last_reg_write << 8);
                break;
            case TMS_NAME_TABLE:
                name_table = last_reg_write * 0x400;
                break;
            case TMS_COLOR_TABLE:
                color_table = last_reg_write * 0x40;
                break;
            case TMS_PATTERN_TABLE:
                pattern_table = last_reg_write * 0x800;
                break;
            case TMS_SPRITE_ATTRIBUTE_TABLE:
                sprite_attribute_table = last_reg_write * 0x80;
                break;
            case TMS_SPRITE_PATTERN_TABLE:
                sprite_pattern_table = last_reg_write * 0x800;
                break;
            case TMS_SCREEN_COLORS:
                screen_colors = last_reg_write;
                break;
            }
        }
        reg_high_byte = 0;
    } else {
        reg_high_byte = 1;
        last_reg_write = data;
    }
    if (control_bits & TMS_M3) {
        color_table &= 0x2000;
        pattern_table &= 0x2000;
    }
}

void tms_save_status(uint8_t data)
{
    reg_high_byte = 0;
    last_status = data;
}

void tms_report()
{
    printf_P(PSTR("Control bits: %04x\n"), control_bits);
    printf_P(PSTR("External video: %c\n"), control_bits & TMS_EXT_VIDEO ? 'Y' : 'N');
    printf_P(PSTR("16K enabled: %c\n"), control_bits & TMS_16K ? 'Y' : 'N');
    printf_P(PSTR("Display enabled: %c\n"), control_bits & TMS_DISPLAY_ENABLE ? 'Y' : 'N');
    printf_P(PSTR("Interrupt enabled: %c\n"), control_bits & TMS_INT_ENABLE ? 'Y' : 'N');
    printf_P(PSTR("Large sprites: %c\n"), control_bits & TMS_SPRITE_32X32 ? 'Y' : 'N');
    printf_P(PSTR("Sprite magnification: %c\n"), control_bits & TMS_SPRITE_MAG ? 'Y' : 'N');
    printf_P(PSTR("Video mode: "));
    if (control_bits & TMS_M1)
        printf_P(PSTR("Text\n"));
    else if (control_bits & TMS_M2)
        printf_P(PSTR("Multicolor\n"));
    else if (control_bits & TMS_M3)
        printf_P(PSTR("Graphics II\n"));
    else
        printf_P(PSTR("Graphics I\n"));
    printf_P(PSTR("Name table: %04x\n"), name_table);
    printf_P(PSTR("Color table: %04x\n"), color_table);
    printf_P(PSTR("Pattern table: %04x\n"), pattern_table);
    printf_P(PSTR("Sprite attribute table: %04x\n"), sprite_attribute_table);
    printf_P(PSTR("Sprite pattern table: %04x\n"), sprite_pattern_table);
}

void tms_fill(uint16_t addr, uint8_t val, uint16_t len)
{
    addr &= 0x3fff;
    addr |= 0x4000;
    io_out(tms_base + 1, addr & 0xff);
    _delay_us(2);
    io_out(tms_base + 1, addr >> 8);
    _delay_us(2);
    for (uint16_t i = 0; i < len; i++) {
        io_out(tms_base, val);
        if (control_bits & 0x40)
            _delay_us(8);
        else
            _delay_us(2);
    }
}

#define tms_loadfont(font) tms_write_P(pattern_table, (font), 0x800);

void tms_bitmap_name()
{
    uint8_t buf[768];
    for (uint16_t i = 0; i < 768; i++)
        buf[i] = i & 0xff;
    tms_write(name_table, buf, 768);
}

void tms_multicolor_name()
{
    uint8_t buf[768];
    uint8_t i = 0;
    for (uint8_t s = 0; s < 6; s++)
        for (uint8_t l = 0; l < 4; l++)
            for (uint8_t b = 0; b < 32; b++)
                buf[i++] = 32 * s + b;
    tms_write(name_table, buf, 768);
}

void tms_init(uint16_t mode)
{
    control_bits = TMS_16K;
    tms_config();
    tms_fill(0, 0, 0x3fff);
    screen_colors = 0xf0;
    name_table = 0x3800;
    color_table = 0x2000;
    pattern_table = 0;
    sprite_attribute_table = 0x3bc0;
    sprite_pattern_table = 0x1800;
    switch (mode) {
    case TMS_TEXT:
        tms_loadfont(font);
        break;
    case TMS_BITMAP:
        tms_bitmap_name();
        break;
    case TMS_MULTICOLOR:
        tms_multicolor_name();
        break;
    }
    control_bits = mode;
    tms_config();
}

uint8_t tms_detect()
{
    uint8_t addresses[] = { 0xbe, 0x98, 0x10, 8 };
    for (uint8_t i = 0; i < sizeof(addresses); i++) {
        tms_base = addresses[i];
        tms_readreg(); // clear vsync bit
        uint8_t reg = tms_readreg(); // confirm that it's cleared
        if (reg & 0x80)
            continue; // if not, check next port
        uint16_t j = 0xffff;
        while (j--) {
            reg = tms_readreg(); // wait for vsync to be set again
            if (reg & 0x80)
                return tms_base; // if set, we found TMS9918A
        }
    }
    return 1; // indicate failure (base address cannot be odd)
}

#endif