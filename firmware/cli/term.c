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
 * @file term.c terminal-related commands
 */

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>

#include "hardware/uart.h"

uint8_t screenwidth = 80;
uint8_t screenheight = 24;

/**
 * Set screen size
 */
void cli_screen(int argc, char *argv[])
{
    uint8_t tmpwidth = 0, tmpheight = 0;
    if (argc >= 2)
        tmpwidth = strtoul(argv[1], NULL, 10);
    if (argc >= 3)
        tmpheight = strtoul(argv[2], NULL, 10);
    if (tmpwidth >= 40)
        screenwidth = tmpwidth;
    if (tmpheight >= 24)
        screenheight = tmpheight;
        printf_P(PSTR("usage: screen <width> <height>\n"));
    printf_P(PSTR("current screen size is %dx%d\n"), screenwidth, screenheight);
}

/**
 * Clear the screen
 */
void cli_cls(int argc, char *argv[])
{
    printf_P(PSTR("\e[0m\e[;H\e[2J"));
}

/**
 * Send an escape sequence
 */
void cli_esc(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <sequence>...\n"), argv[0]);
    }
    for (uint8_t i = 1; i < argc; i++)
        printf_P(PSTR("\e%s"), argv[i]);    // undocumented feature to send escape code
}

/**
 * Send an ascii character
 */
void cli_ascii(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <hex code>...\n"), argv[0]);
    }
    for (uint8_t i = 1; i < argc; i++) {
        char c = strtoul(argv[i], NULL, 16);
        printf_P(PSTR("%c"), c);
    }
}

/**
 * Enable or disable halt
 */
void cli_haltkey(int argc, char *argv[]) {
    if (argc == 2) {
        if (strcmp_P(argv[1], PSTR("off")) == 0)
            watch_key = 0;
        else if (argv[1][0] == '^' && argv[1][1] <= '_' && argv[1][1] >= '@')
            watch_key = argv[1][1] - 64;
        else if (argv[1][0] == '^' && argv[1][1] <= 'z' && argv[1][1] >= 'a')
            watch_key = argv[1][1] - 96;
        else
            watch_key = strtoul(argv[1], NULL, 16) & 0xff;
    }
    if (watch_key == 0)
        printf_P(PSTR("halt key is disabled\n"));
    else {
        printf_P(PSTR("halt key is %02x "), watch_key);
        if (watch_key <= 74)
            printf_P(PSTR("(^%c)\n"), watch_key + 64);
        else if (watch_key >= 33 && watch_key <= 126)
            printf_P(PSTR("(%c)\n"), watch_key);
        else
            printf_P(PSTR("\n"));
    }
}