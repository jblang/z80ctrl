/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2025 J.B. Langston
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
 * @file literals.c Z80 disassembler literal definitions
 */

#include <stdint.h>

#include "util/pgmspace.h"

#include "tokens.h"

// clang-format off

/*
 Literals are grouped by length so that all literals of the same length can be
 packed into a single PROGMEM string and the position of a literal within the
 string can be calculated by multiplying the index times the length.

 On AVR, if the literals weren't grouped by length, each literal would have to
 be stored in a separate PROGMEM string, which is verbose and ugly, and wastes
 an extra byte per literal for the terminating null character.
*/

const char lit1[] PROGMEM =
    // numbers
    "01234567"
    // registers
    "bcdehlair"
    // condition codes
    "zpm";

const char lit2[] PROGMEM = 
    // instructions
    "cp" "di" "ei" "ex" "im" "in" "jp" "jr" "ld" "or" "rl" "rr"
    // registers
    "bc" "de" "hl" "hl" "sp" "af" "ix" "iy" "ix" "iy"
    // condition codes
    "nz" "nc" "po" "pe"
    // prefixes
    "cb" "dd" "ed" "fd";

const char lit3[] PROGMEM = 
    // instructions
    "adc" "add" "and" "bit" "ccf" "cpd" "cpi" "cpl"
    "daa" "dec" "exx" "inc" "ind" "ini" "ldd" "ldi"
    "neg" "nop" "out" "pop" "res" "ret" "rla" "rlc"
    "rld" "rra" "rrc" "rrd" "rst" "sbc" "scf" "set"
    "sla" "sra" "srl" "sll" "sub" "xor"
    // registers
    "ixh" "ixl" "iyh" "iyl" "af'" "(c)"
    // reset addresses
    "00h" "08h" "10h" "18h" "20h" "28h" "30h" "38h"
    // Interrupt modes
    "0/1";

const char lit4[] PROGMEM = 
    // instructions
    "call" "cpdr" "cpir" "djnz" "halt" "indr" "inir" "lddr" "ldir"
    "otdr" "otir" "outd" "outi" "push" "reti" "retn" "rlca" "rrca"
    // undocumented instructions
    "nop8" "noni"
    // registers
    "(bc)" "(de)" "(hl)" "(sp)";

// clang-format on

// 0000-0077 = nonliterals
// 0100-0137 = 1-character literals
// 0140-0177 = 4-character literals
// 0200-0277 = 2-character literals
// 0300-0377 = 3-character literals

// Decode token and copy the corresponding characters into output buffer
uint8_t literal(uint8_t token, char* output)
{
    if (token == _)
        return 0;
    uint8_t len = token >> 6;
    uint8_t i = token & 077;
    const char* input;
    if (len == 1) {
        if (i & 040) { // special case for length 4
            i &= 037;
            len = 4;
            input = &lit4[i * 4];
        } else {
            input = &lit1[i];
        }
    } else {
        i *= len;
        input = (len == 2) ? &lit2[i] : &lit3[i];
    }
    for (i = len; i--;)
        *output++ = pgm_read_byte(input++);
    return len;
}