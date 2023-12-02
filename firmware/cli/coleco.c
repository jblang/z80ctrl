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
 * @file coleco.c coleco command
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fatfs/ff.h"

#include "hardware/bus.h"
#include "hardware/memory.h"
#include "hardware/z80.h"

#include "util/ffwrap.h"
#include "util/pgmspace.h"


/**
 * Load and run a coleco ROM
 */
void cli_coleco(int argc, char *argv[])
{
    uint8_t prevdiv = get_clkdiv();
    uint8_t prevmask = get_haltmask();
    char *coleco_rom = "coleco.rom";
    char *game_rom;
    if (argc > 2)
        coleco_rom = argv[2];
    if (argc > 1) {
        game_rom = argv[1];
    } else {
        printf_P(PSTR("usage: coleco game.rom [coleco.rom]\n"));
        return;
    }
    if (f_stat(game_rom, NULL) != FR_OK) {
        printf_P(PSTR("file not found: %s\n"), game_rom);
        return;
    } else if (f_stat(coleco_rom, NULL) != FR_OK) {
        printf_P(PSTR("file not found: %s\n"), coleco_rom);
        return;
    } 
    mem_loadbin(mem_write_banked, coleco_rom, 0, 0, 0);
    mem_loadbin(mem_write_banked, game_rom, 0x8000, 0, 0);
    set_clkdiv(5);
    set_haltmask(RESET);
    z80_reset(0);
    z80_run();
    set_haltmask(prevmask);
    set_clkdiv(prevdiv);
}