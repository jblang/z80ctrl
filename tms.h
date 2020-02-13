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

#ifndef TMS_H
#define TMS_H

#include <stdint.h>
#include "bus.h"

#ifndef IORQ_OUTPUT
#error "TMS9918 support requires board revision 3 or higher"
#endif


#define TMS_TILE 0
#define TMS_BITMAP 1
#define TMS_MULTICOLOR 2
#define TMS_TEXT 4

typedef struct {
        uint8_t mode : 3;
        uint8_t extvid : 1;
        uint8_t ramsize : 1;
        uint8_t blank : 1;
        uint8_t intenable : 1;
        uint8_t spritesize : 1;
        uint8_t spritemag : 1;
        uint8_t nametable : 4;
        uint8_t colortable;
        uint8_t patterngen : 3;
        uint8_t spriteattr : 7;
        uint8_t spritepat : 3;
        uint8_t textcolor : 4;
        uint8_t bgcolor : 4;
} tms_conf;

typedef struct {
        uint8_t interrupt : 1;
        uint8_t fifthsprite : 1;
        uint8_t coincidence : 1;
        uint8_t spriteno : 5;
} tms_stat;

void tms_configure(tms_conf c);
tms_stat tms_status(void);
uint8_t tms_read(uint16_t addr, uint8_t *buf, uint16_t len);
uint8_t _tms_write(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace);
#define tms_write(addr, buf, len) _tms_write((addr), (buf), (len), 0)
#define tms_write_P(addr, buf, len) _tms_write((addr), (buf), (len), 1)
tms_conf tms_init();
void tms_putchar(char c);


#endif