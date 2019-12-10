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
 * @file util.c Miscellaneous utility functions
 */

#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "ff.h"
#include "bus.h"

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

/**
 * FatFS wrapper to write a single byte to a file, used by stdio library
 */
int fatfs_putchar(char c, FILE * stream)
{
    FIL *fil = stream->udata;
    if (f_write(fil, &c, 1, NULL) != FR_OK)
        return EOF;
    else
        return 0;
}

/**
 * FatFS wrapper to read a single byte from a file, used by stdio library
 */
int fatfs_getchar(FILE * stream)
{
    FIL *fil = stream->udata;
    char c;

    if (f_read(fil, &c, 1, NULL) == FR_OK)
        return c;
    else
        return EOF;
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

char *splitdir(char *path)
{
    char *fs = strrchr(path, '/');
    char *bs = strrchr(path, '\\');
    if  (fs > bs) {
        *fs = '\0';
        return fs + 1;
    } else if (bs != NULL) {
        *bs = '\0';
        return bs + 1;
    }
    return NULL;
}

#define BUFSIZE 8192

FRESULT file_to_mem(FIL *fp, uint16_t base, UINT btr, UINT *br)
{
    uint8_t buf[BUFSIZE];
    FRESULT fr;
    UINT btr1, br1;

    *br = 0;
    do {
        btr1 = btr < BUFSIZE ? btr : BUFSIZE;
        if ((fr = f_read(fp, buf, btr1, &br1)) != FR_OK)
            break;
        mem_write(base, buf, br1);
        *br += br1;
        btr -= br1;
        base += br1;
    } while (btr > 0 && br1 == btr1);
    return fr;
}

FRESULT mem_to_file(FIL *fp, uint16_t base, UINT btw, UINT *bw)
{
    uint8_t buf[BUFSIZE];
    FRESULT fr;
    UINT btw1, bw1;

    *bw = 0;
    do {
        btw1 = btw < BUFSIZE ? btw : BUFSIZE;
        mem_read(base, buf, btw1);
        if ((fr = f_write(fp, buf, btw1, &bw1)) != FR_OK)
            break;
        *bw += bw1;
        btw -= bw1;
        base += bw1;
    } while(btw > 0 && bw1 == btw1);
    return fr;
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

