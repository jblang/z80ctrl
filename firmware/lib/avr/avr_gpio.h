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
 * @file avr_gpio.h lightweight GPIO abstraction for classic and modern AVRs
 */

#ifndef AVR_GPIO_H
#define AVR_GPIO_H

#include <avr/io.h>

#define GPIO_PIN0 (1 << 0)
#define GPIO_PIN1 (1 << 1)
#define GPIO_PIN2 (1 << 2)
#define GPIO_PIN3 (1 << 3)
#define GPIO_PIN4 (1 << 4)
#define GPIO_PIN5 (1 << 5)
#define GPIO_PIN6 (1 << 6)
#define GPIO_PIN7 (1 << 7)
#define GPIO_ALL 0xFF
#define GPIO_NONE 0x00

#if __AVR_ARCH__ < 100

/* Classic AVR */
#define GPIO_OUTPUT(P, V) (DDR ## P) |= (V)
#define GPIO_INPUT(P, V) (DDR ## P) &= ~(V)
#define GPIO_WRITE(P, V) (PORT ## P) = (V)
#define GPIO_SET(P, V) (PORT ## P) |= (V)
#define GPIO_CLEAR(P, V) (PORT ## P) &= ~(V)
#define GPIO_TOGGLE(P, V) (PIN ## P) = (V)
#define GPIO_READ(P) (PIN ## P)
#define GPIO_PULLUP_ON(P, V) (PORT ## P) |= (V)
#define GPIO_PULLUP_OFF(P, V) (PORT ## P) &= ~(V)

#else

/* Modern AVR (xmega registers) */
#define GPIO_OUTPUT(P, V) (PORT ## P).DIRSET = (V)
#define GPIO_INPUT(P, V) (PORT ## P).DIRCLR = (V)
#define GPIO_WRITE(P, V) (PORT ## P).OUT = (V)
#define GPIO_SET(P, V) (PORT ## P).OUTSET = (V)
#define GPIO_CLEAR(P, V) (PORT ## P).OUTCLR = (V)
#define GPIO_TOGGLE(P, V) (PORT ## P).OUTTGL = (V)
#define GPIO_READ(P) ((PORT ## P).IN)
#define GPIO_PULLUP_ON(P, V) (PORT ## P).PINCONFIG = PORT_PULLUPEN_bm; (PORT ## P).PINCTRLSET = (V);
#define GPIO_PULLUP_OFF(P, V) (PORT ## P).PINCONFIG = PORT_PULLUPEN_bm; (PORT ## P).PINCTRLCLR = (V);

#endif

#endif