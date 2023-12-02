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
 * @file tms9918.h TMS9918-related functions
 */

#ifndef HARDWARE_TMS9918_H
#define HARDWARE_TMS9918_H

#include <stdint.h>

#define TMS_TILE 0xc000
#define TMS_BITMAP 0xc002
#define TMS_MULTICOLOR 0xc800
#define TMS_TEXT 0xd000
#define TMS_BLANK 0x8000

void tms_config();
void tms_report();
void tms_read(uint32_t addr, void* buf, uint32_t len);
void tms_write(uint32_t addr, const void* buf, uint32_t len);
void tms_write_P(uint32_t addr, const void* buf, uint32_t len);
void tms_init(uint16_t mode);
void tms_save_status(uint8_t data);
void tms_save_reg(uint8_t data);
uint8_t tms_detect();

#endif