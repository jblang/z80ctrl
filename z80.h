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

#ifndef Z80_H
#define Z80_H

#include <stdint.h>

extern uint16_t bus_watch_start;
extern uint16_t bus_watch_end;
extern uint16_t memrd_watch_start;
extern uint16_t memrd_watch_end;
extern uint16_t memwr_watch_start;
extern uint16_t memwr_watch_end;
extern uint8_t iord_watch_start;
extern uint8_t iord_watch_end;
extern uint8_t iowr_watch_start;
extern uint8_t iowr_watch_end;
extern uint16_t opfetch_watch_start;
extern uint16_t opfetch_watch_end;
extern uint16_t memrd_break_start;
extern uint16_t memrd_break_end;
extern uint16_t memwr_break_start;
extern uint16_t memwr_break_end;
extern uint8_t iord_break_start;
extern uint8_t iord_break_end;
extern uint8_t iowr_break_start;
extern uint8_t iowr_break_end;
extern uint16_t opfetch_break_start;
extern uint16_t opfetch_break_end;

void z80_reset(uint16_t addr);
void z80_status(void);
void z80_run(void);
void z80_debug(uint32_t cycles);

#endif