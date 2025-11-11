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
 * @file immediate.c Z80 disassembler immediate handling
 */
#include <stdint.h>
#include <stdio.h>

#include "util/pgmspace.h"
#include "tokens.h"
#include "disasm.h"

// output an immediate operand in the correct format
uint8_t immediate(uint8_t op, int16_t offset, uint8_t (*input)(), char* output)
{
    char* start = output;

    // wrap indirect operands in parentheses
    uint8_t paren = (op < PAREN);
    if (paren)
        *output++ = '(';

    if (op == BYTE || op == PORT) {
        // retrieve and output immediate operand if specified
        uint8_t imm = input();
        if (imm >= 0xa0) // hex number must begin with 0-9
            *output++ = '0';
        output += sprintf_P(output, PSTR("%02xh"), imm);
    } else if (op == WORD || op == ADDR) {
        uint16_t imm = input();
        imm |= input() << 8;
        if (imm >= 0xa000) // hex number must begin with 0-9
            *output++ = '0';
        output += sprintf_P(output, PSTR("%04xh"), imm);
    } else if (op == DISP) {
        // output relative jump in signed decimal
        int8_t imm = input();
        output += sprintf_P(output, PSTR("%d"), imm);
    } else if (op == IXD || op == IYD) {
        // indirect IX or IY with offset
        uint8_t reg = (op == IXD) ? IX : IY;
        output += literal(reg, output);

        // Read displacement if not pre-read (-256 means not read yet)
        if (offset == -256)
            offset = (int8_t)input();

        // Output signed displacement
        if (offset >= 0) {
            *output++ = '+';
            output += sprintf_P(output, PSTR("%d"), offset);
        } else {
            // Negative displacement already has minus sign from %d
            output += sprintf_P(output, PSTR("%d"), offset);
        }
    }

    // closing parentheses
    if (paren)
        *output++ = ')';

    // return length
    return output - start;
}
