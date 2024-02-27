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

/*
Register concatenation macros with workaround for lack of argument prescan:
https://gcc.gnu.org/onlinedocs/gcc-13.2.0/cpp/Concatenation.html
https://gcc.gnu.org/onlinedocs/gcc-13.2.0/cpp/Argument-Prescan.html#Argument-Prescan
*/
#define GPIO_DDR_P(P) DDR##P
#define GPIO_PIN_P(P) PIN##P
#define GPIO_PORT_P(P) PORT##P

#if __AVR_ARCH__ < 100 /* Classic AVR */

#define GPIO_OUTPUT(P, V) GPIO_DDR_P(P) |= (V)
#define GPIO_INPUT(P, V) GPIO_DDR_P(P) &= ~(V)
#define GPIO_WRITE(P, V) GPIO_PORT_P(P) = (V)
#define GPIO_SET(P, V) GPIO_PORT_P(P) |= (V)
#define GPIO_CLEAR(P, V) GPIO_PORT_P(P) &= ~(V)
#define GPIO_TOGGLE(P, V) GPIO_PIN_P(P) = (V)
#define GPIO_READ(P) GPIO_PIN_P(P)
#define GPIO_PULLUP_ON(P, V) GPIO_PORT_P(P) |= (V)
#define GPIO_PULLUP_OFF(P, V) GPIO_PORT_P(P) &= ~(V)

#else /* Modern AVR (xmega registers) */

#define GPIO_OUTPUT(P, V) GPIO_PORT_P(P).DIRSET = (V)
#define GPIO_INPUT(P, V) GPIO_PORT_P(P).DIRCLR = (V)
#define GPIO_WRITE(P, V) GPIO_PORT_P(P).OUT = (V)
#define GPIO_SET(P, V) GPIO_PORT_P(P).OUTSET = (V)
#define GPIO_CLEAR(P, V) GPIO_PORT_P(P).OUTCLR = (V)
#define GPIO_TOGGLE(P, V) GPIO_PORT_P(P).OUTTGL = (V)
#define GPIO_READ(P) (GPIO_PORT_P(P).IN)
#define GPIO_PULLUP_ON(P, V)                              \
    do {                                                  \
        GPIO_PORT_P(P).PINCONFIG = GPIO_PORT_PULLUPEN_bm; \
        GPIO_PORT_P(P).PINCTRLSET = (V);                  \
    } while (0);
#define GPIO_PULLUP_OFF(P, V)                             \
    do {                                                  \
        GPIO_PORT_P(P).PINCONFIG = GPIO_PORT_PULLUPEN_bm; \
        GPIO_PORT_P(P).PINCTRLCLR = (V);                  \
    } while (0);

#endif

#endif