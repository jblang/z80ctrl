/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
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
 * @file util.c Miscellaneous utility functions
 */

#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

/**
 * Look up a text string by index from a NULL-separated PROGMEM array
 */
PGM_P strlookup(PGM_P str, uint32_t index)
{
    PGM_P p;
    uint32_t i;

    for (p = str, i = 0; i != index; i++)
        while(pgm_read_byte(p++))
            ;

    return p;
}

void config_timer(uint8_t timer, uint8_t prescaler)
{
    switch (timer) {
        case 0:
            TIMSK0 = 0;
            TCCR0A = 0;
            TCCR0B = prescaler & 7;
            break;
        case 1:
            TIMSK1 = 0;
            TCCR1A = 0;
            TCCR1B = prescaler & 7;
            break;
        case 2:
            TIMSK2 = 0;
            TCCR2A = 0;
            TCCR2B = prescaler & 7;
            break;
        case 3:
            TIMSK3 = 0;
            TCCR3A = 0;
            TCCR3B = prescaler & 7;
            break;
    }
}

uint16_t get_tcnt(uint8_t timer)
{
    if (timer == 0) {
        return TCNT0;
    } else if (timer == 2) {
        return TCNT2;
    }

    uint8_t sreg = SREG;
    uint16_t value = 0;
    cli();
    if (timer == 1)
        value = TCNT1;
    else if (timer == 3)
        value = TCNT3;
    SREG = sreg;
    return value;
}

void set_tcnt(uint8_t timer, uint16_t value)
{
    if (timer == 0) {
        TCNT0 = value;
        return;
    } else if (timer == 2) {
        TCNT2 = value;
        return;
    }

    uint8_t sreg = SREG;
    cli();
    if (timer == 1)
        TCNT1 = value;
    else if (timer == 3)
        TCNT3 = value;
    SREG = sreg;
}

uint8_t clibuf[256];

void save_cli(int argc, char *argv[])
{
    memset(clibuf, 0, 256);
    clibuf[0] = argc;
    uint8_t *next = clibuf + 1;

    for (int i = 0; i < argc; i++)
    {
        strcpy(next, argv[i]);
        next += strlen(argv[i]) + 1;
    }
}
