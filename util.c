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
#include <stdio.h>

#include "util.h"
#include "ff.h"

/**
 * Look up a text string by index from a NULL-separated PROGMEM array
 */
PGM_P strlookup(PGM_P str, uint32_t index)
{
    PGM_P p;
    uint32_t i;

    for (p = str, i = 0; i != index && pgm_read_byte(p); i++)
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