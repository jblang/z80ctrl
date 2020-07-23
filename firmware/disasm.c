/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2020 J.B. Langston
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

#include "disasm.h"
#include "bus.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**
 * @file disasm.c Z80 disassembler
 * 
 * This disassembler decodes Z80 instructions using techniques described 
 * in http://www.z80.info/decoding.htm.
 */

/*
 Tokens for each assembly mnemonic and operand are defined as follows:

 - Tokens between 0 and 077 (octal) are one character literals
 - Tokens between 0100 and 0177 are two character literals
 - Tokens between 0200 and 0277 are three character literals
 - Tokens between 0300 and 0367 are four character literals
 - Tokens between 0370 and 0377 are special non-literal values

 Thus the top 2 bits encode the literal's length and the bottom 6 bits
 encode the index of the literal within its respective length class.

 Literals are grouped by length so that all literals of the same length can
 be packed into a single PROGMEM string and the position of a literal within
 the string can be calculated by multiplying the index times the length.

 If the literals weren't grouped by length, they would each have to be
 stored in a separate PROGMEM string, which is verbose and ugly, and wastes
 an extra byte per literal for the terminating null character.

 The `literal` function is provided to copy the literal string associated 
 with the specified token into a buffer.
*/

// constant starting values for different classes of tokens
enum {LIT1 = 0, LIT2 = 0100, LIT3 = 0200, LIT4 = 0300, NONLIT = 0370};

// 1-character literals
enum {
    // numbers
    N0, N1, N2, N3, N4, N5, N6, N7,
    // registers
    B, C, D, E, H, L, A, I, R,
    // condition codes
    Z, P, M, 
};

const char lit1[] PROGMEM = 
    "01234567"
    "bcdehlair"
    "zpm";

// 2-character literals
enum {
    // instructions
    CP = LIT2, DI, EI, EX, IM, IN, JP, JR, LD, OR, RL, RR, 
    // registers
    BC, DE, HL, NSHL, SP_, AF, IX, IY, IXI, IYI,
    // condition codes
    NZ, NC, PO, PE
};

const char lit2[] PROGMEM = 
    "cp" "di" "ei" "ex" "im" "in" "jp" "jr" "ld" "or" "rl" "rr"
    "bc" "de" "hl" "hl" "sp" "af" "ix" "iy" "ix" "iy"
    "nz" "nc" "po" "pe";

// 3-character literals
enum {
    // instructions
    ADC_ = LIT3, ADD, AND, BIT, CCF, CPD, CPI, CPL, 
    DAA, DEC, EXX, INC, IND, INI, LDD, LDI, 
    NEG, NOP, OUT, POP, RES, RET, RLA, RLC,
    RLD, RRA, RRC, RRD, RST, SBC, SCF, SET, 
    SLA, SRA, SRL, SLL, SUB, XOR, 
    // registers
    IXH, IXL, IYH, IYL, AFP, CI,
    // reset addresses
    H00, H08, H10, H18, H20, H28, H30, H38,
    // Interrupt modes
    N01
};

const char lit3[] PROGMEM = 
    "adc" "add" "and" "bit" "ccf" "cpd" "cpi" "cpl"
    "daa" "dec" "exx" "inc" "ind" "ini" "ldd" "ldi"
    "neg" "nop" "out" "pop" "res" "ret" "rla" "rlc"
    "rld" "rra" "rrc" "rrd" "rst" "sbc" "scf" "set"
    "sla" "sra" "srl" "sll" "sub" "xor"
    "ixh" "ixl" "iyh" "iyl" "af'" "(c)"
    "00h" "08h" "10h" "18h" "20h" "28h" "30h" "38h"
    "0/1";

// 4-character literals
enum {
    // instructions
    CALL = LIT4, CPDR, CPIR, DJNZ, HALT_, INDR, INIR, LDDR, LDIR,
    OTDR, OTIR, OUTD, OUTI, PUSH, RETI, RETN, RLCA, RRCA, NOP8,
    // registers
    BCI, DEI, HLI, SPI
};

const char lit4[] PROGMEM = 
    "call" "cpdr" "cpir" "djnz" "halt" "indr" "inir" "lddr" "ldir"
    "otdr" "otir" "outd" "outi" "push" "reti" "retn" "rlca" "rrca" "nop8"
    "(bc)" "(de)" "(hl)" "(sp)";

// nonliteral tokens
enum {
    _ = NONLIT, REL, BYTE, BYTEI, WORD, WORDI
};

/*
 Lookup table for registers and opcodes.  Instructions that are highly 
 regular such as `aluop reg` and `ld reg, reg` are handled specially
 to reduce the size of the lookup tables.  Some columns in the lookup
 table are regular and could be encoded more compactly if special cased
 but doing so would add more bytes of code than it would save in the
 lookup tables.
*/

// Misc ops from 0x00 to 0x3F
const uint8_t x0ops[] PROGMEM = {
    NOP, _, _,          LD, BC, WORD,       LD, BCI, A,         INC, BC, _,         INC, B, _,          DEC, B, _,          LD, B, BYTE,        RLCA, _, _,
    EX, AF, AFP,        ADD, HL, BC,        LD, A, BCI,         DEC, BC, _,         INC, C, _,          DEC, C, _,          LD, C, BYTE,        RRCA, _, _,
    DJNZ, REL, _,       LD, DE, WORD,       LD, DEI, A,         INC, DE, _,         INC, D, _,          DEC, D, _,          LD, D, BYTE,        RLA, _, _,
    JR, REL, _,         ADD, HL, DE,        LD, A, DEI,         DEC, DE, _,         INC, E, _,          DEC, E, _,          LD, E, BYTE,        RRA, _, _,
    JR, NZ, REL,        LD, HL, WORD,       LD, WORDI, HL,      INC, HL, _,         INC, H, _,          DEC, H, _,          LD, H, BYTE,        DAA, _, _,
    JR, Z, REL,         ADD, HL, HL,        LD, HL, WORDI,      DEC, HL, _,         INC, L, _,          DEC, L, _,          LD, L, BYTE,        CPL, _, _,
    JR, NC, REL,        LD, SP_, WORD,      LD, WORDI, A,       INC, SP_, _,        INC, HLI, _,        DEC, HLI, _,        LD, HLI, BYTE,      SCF, _, _,
    JR, C, REL,         ADD, HL, SP_,       LD, A, WORDI,       DEC, SP_, _,        INC, A, _,          DEC, A, _,          LD, A, BYTE,        CCF, _, _

};

// Misc ops from 0xC0 to 0xFF
const uint8_t x3ops[] PROGMEM = {
    RET, NZ, _,         POP, BC, _,         JP, NZ, WORD,       JP, WORD, _,        CALL, NZ, WORD,     PUSH, BC, _,        ADD, A, BYTE,       RST, H00, _,
    RET, Z, _,          RET, _, _,          JP, Z, WORD,        _, _, _,            CALL, Z, WORD,      CALL, WORD, _,      ADC_, A, BYTE,      RST, H08, _,
    RET, NC, _,         POP, DE, _,         JP, NC, WORD,       OUT, BYTEI, A,      CALL, NC, WORD,     PUSH, DE, _,        SUB, BYTE, _,       RST, H10, _,
    RET, C, _,          EXX, _, _,          JP, C, WORD,        IN, A, BYTEI,       CALL, C, WORD,      _, _, _,            SBC, A, BYTE,       RST, H18, _,
    RET, PO, _,         POP, HL, _,         JP, PO, WORD,       EX, SPI, HL,        CALL, PO, WORD,     PUSH, HL, _,        AND, BYTE, _,       RST, H20, _,
    RET, PE, _,         JP, HLI, _,         JP, PE, WORD,       EX, DE, NSHL,       CALL, PE, WORD,     _, _, _,            XOR, BYTE, _,       RST, H28, _,
    RET, P, _,          POP, AF, _,         JP, P, WORD,        DI, _, _,           CALL, P, WORD,      PUSH, AF, _,        OR, BYTE, _,        RST, H30, _,
    RET, M, _,          LD, SP_, HL,        JP, M, WORD,        EI, _, _,           CALL, M, WORD,      _, _, _,            CP, BYTE, _,        RST, H38, _
};

// Misc ED-prefixed ops from 0x40 to 0x7F
const uint8_t edx1ops[] PROGMEM = {
    IN, B, CI,          OUT, CI, B,         SBC, HL, BC,        LD, WORDI, BC,      NEG, _, _,          RETN, _, _,         IM, N0, _,         LD, I, A, 
    IN, C, CI,          OUT, CI, C,         ADC_, HL, BC,       LD, BC, WORDI,      NEG, _, _,          RETI, _, _,         IM, N01, _,        LD, R, A,
    IN, D, CI,          OUT, CI, D,         SBC, HL, DE,        LD, WORDI, DE,      NEG, _, _,          RETN, _, _,         IM, N1, _,         LD, A, I, 
    IN, E, CI,          OUT, CI, E,         ADC_, HL, DE,       LD, DE, WORDI,      NEG, _, _,          RETN, _, _,         IM, N2, _,         LD, A, R, 
    IN, H, CI,          OUT, CI, H,         SBC, HL, HL,        LD, WORDI, HL,      NEG, _, _,          RETN, _, _,         IM, N0, _,         RRD, _, _, 
    IN, L, CI,          OUT, CI, L,         ADC_, HL, HL,       LD, HL, WORDI,      NEG, _, _,          RETN, _, _,         IM, N01, _,        RLD, _, _, 
    IN, _, CI,          OUT, CI, 0,         SBC, HL, SP_,       LD, WORDI, SP_,     NEG, _, _,          RETN, _, _,         IM, N1, _,         NOP, _, _, 
    IN, A, CI,          OUT, CI, A,         ADC_, HL, SP_,      LD, SP_, WORDI,     NEG, _, _,          RETN, _, _,         IM, N2, _,         NOP, _, _
};

// Registers
const uint8_t reg[] PROGMEM = {B, C, D, E, H, L, HLI, A};

// ALU operations
const uint8_t aluops[] PROGMEM = {ADD, ADC_, SUB, SBC, AND, XOR, OR, CP};

// Extended rotation operations
const uint8_t rotops[] PROGMEM = {RLC, RRC, RL, RR, SLA, SRA, SLL, SRL};

// Bit operations
const uint8_t bitops[] PROGMEM = {_, BIT, RES, SET};

// Block operations
const uint8_t blockops[] PROGMEM = {
    LDI, CPI, INI, OUTI, 
    LDD, CPD, IND, OUTD, 
    LDIR, CPIR, INIR, OTIR, 
    LDDR, CPDR, INDR, OTDR, 
};

// Decode token and copy the corresponding characters into output buffer
uint8_t literal(uint8_t token, char *output)
{
    if (token >= NONLIT)
        return 0;
    const char *literals[] = {lit1, lit2, lit3, lit4};
    uint8_t len = token >> 6;
    const char *input = literals[len] + (token & 077) * ++len;
    uint8_t i = len;
    while (i--)
        *output++ = pgm_read_byte(input++);
    return len;
}

// Output an operand in the correct format
uint8_t operand(uint8_t op, int8_t offset, uint8_t (*input)(), char *output)
{
    char *start = output;

    // Return if no operand
    if (op == _) return 0;
    
    // Skip space
    *output++ = ' ';

    // Wrap indirect operands in parentheses
    uint8_t paren = (op == WORDI || op == BYTEI || op == IXI || op == IYI);
    if (paren)
        *output++ = '(';

    if (op == BYTE || op == BYTEI || op == WORD || op == WORDI) {
        // Retrieve and output immediate operand if specified
        uint16_t imm = input();
        if (op == WORD || op == WORDI)
            imm |= input() << 8;
        output += sprintf_P(output, PSTR("0%04xh"), imm);
    } else if (op == REL) {
        // Output relative jump in signed decimal
        int8_t imm = input();
        output += sprintf_P(output, PSTR("%d"), imm);
    } else if (op == IXI || op == IYI) {
        // Indirect IX or IY with offset
        output += literal(op, output);
        if (offset >= 0) {
            *output++ = '+';
            output += sprintf_P(output, PSTR("%d"), offset);
        }
    } else {
        // Literal
        output += literal(op, output);
    }

    // Closing parentheses
    if (paren)
        *output++ = ')';

    // Return length
    return output - start;
}

// Substitute IX or IY for HL as appropriate
const uint8_t regsub(uint8_t reg, uint8_t submode)
{
    if (submode == IX) {
        if (reg == HL) 
            reg = IX;
        else if (reg == HLI) 
            reg = IXI;
        else if (reg == H) 
            reg = IXH;
        else if (reg == L) 
            reg = IXL;
    } else if (submode == IY) {
        if (reg == HL) 
            reg = IY;
        else if (reg == HLI) 
            reg = IYI;
        else if (reg == H) 
            reg = IYH;
        else if (reg == L) 
            reg = IYL;
    }
    return reg;
}

#define LOOKUP(table, index) pgm_read_byte(&table[index])

// Disassemble an instruction
uint8_t disasm(uint8_t (*input)(), char *output)
{
    // Consume any number of 0xDD and 0xFD prefix bytes and set 
    // IX/IY substituion mode according to last byte encountered
    uint8_t opcode = 0;
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
    int8_t offset = -1;
    if (opcode == 0xED) {
        prefix = 0xED;
        submode = HL;            // Address register for 0xED prefix is always HL
        opcode = input();
    } else if (opcode == 0xCB) {
        prefix = 0xCB;
        // 0xDDCB and 0xFDCB instructions have a displacement byte before the opcode
        if (submode != HL)
            offset = input();
        opcode = input();
    }

    // Slice the opcode into xxyyyzzz (see http://www.z80.info/decoding.htm)
    uint8_t x = opcode >> 6;                // x = opcode[7:6]
    uint8_t y = (opcode >> 3) & 7;          // y = opcode[5:3]
    uint8_t z = opcode & 7;                 // z = opcode[2:0]
    uint8_t rz = LOOKUP(reg, z);

    // Initialize instruction and operands to blank
    uint8_t instr = _;
    uint8_t op1 = _;
    uint8_t op2 = _;

    // Decode opcode into instruction and operand tokens
    if (prefix == 0xCB) {
        // CB-prefixed opcodes
        if (x == 0) {
            // Rotate/shift ops
            instr = LOOKUP(rotops, y);
            op1 = rz;
        } else {
            // Bit ops
            instr = LOOKUP(bitops, x);
            op1 = y;
            op2 = rz;
        }
    } else if (prefix == 0xED) {
        // ED-prefixed opcodes
        if (x == 1) {
            // misc instructions
            uint8_t opindex = (opcode - 0x40) * 3;
            instr = LOOKUP(edx1ops, opindex);
            op1 = LOOKUP(edx1ops, opindex + 1);
            op2 = LOOKUP(edx1ops, opindex + 2);
        } else if (x == 2 && z <= 3 && y >= 4) {
            // block operations
            instr = LOOKUP(blockops, z + (y - 4) * 4);
        } else {
            // Undocumented 8 T-state nop
            instr = NOP8;
        }
    } else {
        // un-prefixed opcodes
        if (x == 0) {
            // misc instructions
            uint8_t opindex = opcode * 3;
            instr = LOOKUP(x0ops, opindex);
            op1 = LOOKUP(x0ops, opindex + 1);
            op2 = LOOKUP(x0ops, opindex + 2);
        } else if (x == 1) {
            // ld reg, reg instructions
            if (opcode == 0x76) {
                instr = HALT_;
            } else {
                instr = LD;
                op1 = LOOKUP(reg, y);
                op2 = rz;
            }
        } else if (x == 2) {
            // alu reg instructions
            instr = LOOKUP(aluops, y);
            if (y <= 3 && y != 2)
                op1 = A;
            op2 = rz;
        } else if (x == 3) {
            // misc instructions
            uint8_t opindex = (opcode - 0xC0) * 3;
            instr = LOOKUP(x3ops, opindex);
            op1 = LOOKUP(x3ops, opindex + 1);
            op2 = LOOKUP(x3ops, opindex + 2);
        }
    }

    // Handle IX/IY register substitution
    if (prefix == 0 && submode != HL && instr != JP && (op1 == HLI || op2 == HLI))
        offset = input();
    if (op2 != HLI)
        op1 = regsub(op1, submode);
    if (op1 != IXI && op1 != IYI)
        op2 = regsub(op2, submode);

    // Handle undocumented load + bit/rotate instructions
    if (prefix == 0xCB && submode != HL && x != 1 && z != 6) {
        output += literal(LD, output);
        *output++ = ' ';
        output += literal(rz, output);
        *output++ = ',';
        *output++ = ' ';
    }

    // Output instruction mnemonic
    output += literal(instr, output);
    output += operand(op1, offset, input, output);
    if (op1 != _ && op2 != _)
        *output++ = ',';
    output += operand(op2, offset, input, output);
    *output = '\0';
}

static uint8_t disasm_index = 0;   /**< index of next byte within 256 byte buffer */
static uint32_t disasm_addr = 0;   /**< address of next chunk to read from external RAM */
static uint8_t *disasm_buf;        /**< pointer to disassembly buffer */

static uint8_t instr_bytes[8];     /**< bytes contained in the current instruction */
static uint8_t instr_length = 0;   /**< length of the current construction */

/**
 * Return next byte for instruction from memory
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
        printf_P(PSTR("%05lx "), disasm_addr + base_addr);
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