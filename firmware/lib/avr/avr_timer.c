/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation argvs (the "Software"),
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
 * @file timer.c AVR timer functions
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "avr_timer.h"

#if __AVR_ARCH__ < 100 // Classic AVRs

timer_callback_t timer_callback;

void timer_callback_start(uint32_t hz, timer_callback_t callback)
{
    OCR0A = F_CPU / 1024 / hz - 1;
    TCCR0A = (1 << WGM01);
    TCCR0B = 0b101;
    TIMSK0 = (1 << OCIE0A);
    timer_callback = callback;
    sei();
}

ISR(TIMER0_COMPA_vect)
{
    timer_callback();
}

void timer_pwm_start(uint8_t oca, uint8_t ocb)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = oca;
    OCR2B = ocb;
}

void timer_pwm_stop(void)
{
    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = 0;
    OCR2B = 0;
}

#else // Modern AVRs (xmega register architecture)

void timer_callback_init(uint32_t hz, timer_callback_t callback)
{
}

void timer_pwm_start(uint8_t oca, uint8_t ocb)
{
}

void timer_pwm_stop()
{
}

#endif