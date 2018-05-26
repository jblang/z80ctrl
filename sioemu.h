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
 * @file sioemu.h Serial I/O emulation
 */

#ifndef SIOEMU_H
#define SIOEMU_H

#define SIO0_STATUS 0x10
#define SIO0_DATA 0x11
#define SIO1_STATUS 0x12
#define SIO1_DATA 0x13
#define SIOA_CONTROL 0x80
#define SIOA_DATA 0x81
#define SIOB_CONTROL 0x82
#define SIOB_DATA 0x83

/**
 * Utility macros to generate SIO status register values
 */
#define ACIA_STATUS(u) ((((uart_testtx(z80_uart[(u)]) == 0) << 1) & 0x2) | ((uart_testrx(z80_uart[(u)]) > 0) & 0x1))
#define ZSIO_STATUS(u) ((1 << 3) | (1  << 5) | (((uart_testtx(z80_uart[(u)]) == 0) << 2) & 0x4) | ((uart_testrx(z80_uart[(u)]) > 0) & 0x1))

extern uint8_t z80_uart[];

#endif