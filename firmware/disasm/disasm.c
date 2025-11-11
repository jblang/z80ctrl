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
 * FITNESS FOR a PARTICULAR PURPOSE AND NONINFRINGEMENT. in NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER in AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, out OF OR in CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS in THE SOFTWARE.
 */

/**
 * @file disasm.c Z80 disassembler
 *
 * This disassembler decodes Z80 instructions using techniques described
 * in http://www.z80.info/decoding.htm.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/memory.h"

#include "util/pgmspace.h"

#include "disasm.h"
#include "tokens.h"

// Forward declaration from lookup.c
extern instr_t decode(uint8_t prefix, uint8_t opcode);

// substitute IX or IY for HL as appropriate
// sub_h_l: if false, don't substitute H/L (used when instruction has (IX+d) operand)
uint8_t regsub(uint8_t reg, uint8_t submode, uint8_t sub_h_l)
{
    if (submode == IX) {
        if (reg == HL)
            reg = IX;
        else if (reg == HLI)
            reg = IXD;  // Use IXD (immediate with displacement) not IXI (literal)
        else if (sub_h_l && reg == H)
            reg = IXH;
        else if (sub_h_l && reg == L)
            reg = IXL;
    } else if (submode == IY) {
        if (reg == HL)
            reg = IY;
        else if (reg == HLI)
            reg = IYD;  // Use IYD (immediate with displacement) not IYI (literal)
        else if (sub_h_l && reg == H)
            reg = IYH;
        else if (sub_h_l && reg == L)
            reg = IYL;
    }
    return reg;
}

// Output an operand (either literal, immediate, or digit)
uint8_t operand(uint8_t op, int16_t offset, uint8_t (*input)(), char* output)
{
    char* start = output;

    // Return if no operand
    if (op == _)
        return 0;

    // Skip space
    *output++ = ' ';

    // Check if this is an immediate operand
    if (TOKTYPE(op) == IMMEDIATE) {
        output += immediate(op, offset, input, output);
    } else if (op >= 0 && op <= 7) {
        // It's a raw digit (0-7) from Y_DIGIT token type
        *output++ = '0' + op;
    } else {
        // It's a literal token
        // IXI and IYI need parentheses (for JP (IX), EX (SP), IX, etc.)
        if (op == IXI || op == IYI) {
            *output++ = '(';
            output += literal(op, output);
            *output++ = ')';
        } else {
            output += literal(op, output);
        }
    }

    return output - start;
}

// Disassemble an instruction
uint8_t disasm(uint8_t (*input)(), char* output)
{
    char* start = output;

    // Consume any number of 0xDD and 0xFD prefix bytes and set
    // IX/IY substituion mode according to last byte encountered
    uint8_t opcode;
    uint8_t submode = HL;
    for (;;) {
        opcode = input();
        if (opcode == 0xDD)
            submode = IX;
        else if (opcode == 0xFD)
            submode = IY;
        else
            break;
    }

    // Handle 0xED/0xCB prefix
    uint8_t prefix = 0;
    int16_t offset = -256;  // displacement for IX/IY ops (-256 = not read yet)
    if (opcode == 0xED) {
        prefix = 0xED;
        submode = HL; // Address register for 0xED prefix is always HL
        opcode = input();
    } else if (opcode == 0xCB) {
        prefix = 0xCB;
        // 0xDDCB and 0xFDCB instructions have a displacement byte before the opcode
        if (submode != HL)
            offset = (int8_t)input();  // Cast to int8_t for sign extension
        opcode = input();
    }

    // Decode instruction
    instr_t i = decode(prefix, opcode);

    // Handle IX/IY register substitution
    // Note: Don't substitute H/L when instruction has (IX+d)/(IY+d) operand
    uint8_t has_indexed = (i.p1 == HLI || i.p2 == HLI);
    i.p1 = regsub(i.p1, submode, !has_indexed);
    i.p2 = regsub(i.p2, submode, !has_indexed);

    // For JP (IX/IY) and EX (SP), IX/IY: no displacement byte exists
    // Change IXD/IYD back to IXI/IYI
    if (i.op == JP || (i.op == EX && i.p1 == SPI)) {
        if (i.p1 == IXD) i.p1 = IXI;
        if (i.p1 == IYD) i.p1 = IYI;
        if (i.p2 == IXD) i.p2 = IXI;
        if (i.p2 == IYD) i.p2 = IYI;
    }

    // Handle A operand on ALU ops
    // For ADD, ADC, SBC: need explicit A as first operand
    // For other ALU ops (SUB, AND, XOR, OR, CP): A is implicit
    uint8_t y = (opcode >> 3) & 7;
    uint8_t x = opcode >> 6;
    uint8_t z = opcode & 7;

    if (prefix == 0 && x == 2) {
        // ALU operations with register operands (x=2, z=0-7)
        if (y == 0 || y == 1 || y == 3) {
            // ADD (y=0), ADC (y=1), SBC (y=3) - need explicit A
            i.p2 = i.p1;  // Move register to p2
            i.p1 = A;     // Set p1 to A
        }
        // For y=2 (SUB), y=4 (AND), y=5 (XOR), y=6 (OR), y=7 (CP), implicit A
    } else if (prefix == 0 && x == 3 && z == 6) {
        // ALU operations with immediate operands (x=3, z=6)
        // ADD A, n (0xC6) - y=0
        // ADC A, n (0xCE) - y=1
        // SBC A, n (0xDE) - y=3
        // All other immediate ALU ops have implicit A
        if (y != 0 && y != 1 && y != 3) {
            // SUB, AND, XOR, OR, CP with immediate - remove the A
            i.p1 = i.p2;  // Move immediate to p1
            i.p2 = _;     // Clear p2
        }
    }

    // Output instruction mnemonic
    output += literal(i.op, output);
    output += operand(i.p1, offset, input, output);
    if (i.p1 != _ && i.p2 != _)
        *output++ = ',';
    output += operand(i.p2, offset, input, output);
    *output = '\0';

    return output - start;
}

static uint8_t disasm_index = 0; /**< index of next N within 256 N buffer */
static uint32_t disasm_addr = 0; /**< address of next chunk to read from external RAM */
static uint8_t* disasm_buf;      /**< pointer to disassembly buffer */

static uint8_t instr_bytes[8];   /**< bytes contained in the current instruction */
static uint8_t instr_length = 0; /**< length of the current construction */

/**
 * Return next N for instruction from memory
 */
uint8_t disasm_next_byte()
{
    if (disasm_index == 0)
        mem_read(disasm_addr, disasm_buf, 256);
    disasm_addr++;
    instr_bytes[instr_length] = disasm_buf[disasm_index++];
    return instr_bytes[instr_length++];
}

/**
 * Disassemble instructions from external memory to console
 */
void disasm_mem(uint32_t start, uint32_t end)
{
    char mnemonic[64];
    uint8_t buf[256];
    uint8_t i;

    disasm_addr = start;
    disasm_index = 0;
    disasm_buf = buf;

    while (start <= disasm_addr && disasm_addr <= end) {
        instr_length = 0;
        printf_P(PSTR("%05lx "), disasm_addr);
        disasm(disasm_next_byte, mnemonic);
        for (i = 0; i < instr_length; i++) {
            printf_P(PSTR("%02x "), instr_bytes[i]);
        }
        i = 4 - instr_length;
        while (i--)
            printf_P(PSTR("   "));
        for (i = 0; i < instr_length; i++) {
            if (0x20 <= instr_bytes[i] && instr_bytes[i] <= 0x7e)
                printf_P(PSTR("%c"), instr_bytes[i]);
            else
                printf_P(PSTR("."));
        }
        i = 4 - instr_length;
        while (i--)
            printf_P(PSTR(" "));
        printf_P(PSTR(" %s\n"), mnemonic);
    }
}