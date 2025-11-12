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
 * @file decode.c Z80 disassembler instruction decoding
 */

#include <stdint.h>

#include "util/pgmspace.h"

#include "tokens.h"

// Reference: http://www.z80.info/decoding.htm

//////////////// token lookups ////////////////

// clang-format off

// multiple 8-element lookup tables packed into a single array
// Order MUST match the Y_TOKEN enum order: Y_REG8=0, Y_CC=1, Y_RST=2, Y_IM=3, Y_ALU=4, Y_ROT=5, Y_FLAG=6
const uint8_t tokens[] PROGMEM = {
    // Y_REG8 (0): 8-bit registers
    B, C, D, E, H, L, HLI, A,
    // Y_CC (1): condition codes
    NZ, Z, NC, C, PO, PE, P, M,
    // Y_RST (2): rst parameters
    H00, H08, H10, H18, H20, H28, H30, H38,
    // Y_IM (3): interrupt modes
    N0, N01, N1, N2, N0, N01, N1, N2,
    // Y_ALU (4): alu operations
    ADD, ADC_, SUB, SBC, AND, XOR, OR, CP,
    // Y_ROT (5): bit rotation operations
    RLC, RRC, RL, RR, SLA, SRA, SLL, SRL,
    // Y_FLAG (6): accumulator/flag operations
    RLCA, RRCA, RLA, RRA, DAA, CPL, SCF, CCF,
    // P_REG16 (7): 16-bit register selected by p field
    BC, DE, HL, SP_ // last one is only 4 bytes. sneaky!
};

// block operations
const uint8_t blockops[] PROGMEM = {
/* y z> 0       1       2       3 */
/* 4 */ LDI,    CPI,    INI,    OUTI, 
/* 5 */ LDD,    CPD,    IND,    OUTD, 
/* 6 */ LDIR,   CPIR,   INIR,   OTIR, 
/* 7 */ LDDR,   CPDR,   INDR,   OTDR, 
};

// clang-format on

// lookup token of specified type for opcode
uint8_t t_lookup(uint8_t tok, uint8_t y, uint8_t z)
{
    uint8_t p = y >> 1;
    uint8_t q = y & 1; // q is bit 0 of y
    uint8_t offset = tok;
    uint8_t index = y;

    switch (tok) {
    case Y_DIGIT:
        return y;
    case Q_INC_DEC:
        return q ? DEC : INC;
    case Q_SBC_ADC:
        return q ? ADC_ : SBC;
    case Y_RETI_RETN:
        return y == 1 ? RETI : RETN;
    case YZ_BLOCK:
        return (y > 3 && z < 4) ? LOOKUP(blockops, z + (y - 4) * 4) : NONI;
    case P_REG16:
        index = p;
        break;
    case Z_REG8:
        offset = Y_REG8;
        index = z;
        break;
    default:
        if (TOKTYPE(tok) != Y_TOKEN)
            return tok; // Literal tokens pass through
    }
    return LOOKUP(tokens, offset * 8 + index);
}

//////////////// Instruction LUTs ////////////////

// Z-based 8-instruction lookup tables (indexed by z field)
// Order MUST match Z_INSTR enum order: Z_0YZ, Z_3YZ, Z_ED_1YZ
const instr_t z_instrs[] PROGMEM = {
    // Z_0YZ (offset 0): unprefixed instructions from 000-077
    { Y_JR_MISC, _, _ },
    { Y_LD_ADD_16, _, _ },
    { Y_LD_IND, _, _ },
    { Q_INC_DEC, P_REG16, _ },
    { INC, Y_REG8, _ },
    { DEC, Y_REG8, _ },
    { LD, Y_REG8, BYTE },
    { Y_FLAG, _, _ },

    // Z_3YZ (offset 8): unprefixed instructions from 300-377
    { RET, Y_CC, _ },
    { Y_POP_MISC, _, _ },
    { JP, Y_CC, WORD },
    { Y_MISC, _, _ },
    { CALL, Y_CC, WORD },
    { Y_PUSH_CALL, _, _ },
    { Y_ALU, A, BYTE },
    { RST, Y_RST, _ },

    // Z_ED_1YZ (offset 16): ED-prefixed instructions from 100-177
    { IN, Y_REG8, CI },
    { OUT, CI, Y_REG8 },
    { Q_SBC_ADC, HL, P_REG16 },
    { Y_LD16_IMM, _, _ },
    { NEG, _, _ },
    { Y_RETI_RETN, _, _ },
    { IM, Y_IM, _ },
    { Y_LD_IR_ROT, _, _ }
};

// Y-based 8-instruction lookup tables (indexed by y field)
// Order MUST match Y_INSTR enum order: Y_JR_MISC through Y_LD_IR_ROT
const instr_t y_instrs[] PROGMEM = {
    // 0y0: Y_JR_MISC (offset 0): relative jumps and miscellaneous
    { NOP, _, _ },
    { EX, AF, AFP },
    { DJNZ, DISP, _ },
    { JR, DISP, _ },
    { JR, NZ, DISP },
    { JR, Z, DISP },
    { JR, NC, DISP },
    { JR, C, DISP },

    // 0y1: Y_LD_ADD_16 (offset 8): 16-bit immediate load / ADD HL operations
    { LD, BC, WORD },
    { ADD, HL, BC },
    { LD, DE, WORD },
    { ADD, HL, DE },
    { LD, HL, WORD },
    { ADD, HL, HL },
    { LD, SP_, WORD },
    { ADD, HL, SP_ },

    // 0y2: Y_LD_IND (offset 16): indirect load operations
    { LD, BCI, A },
    { LD, A, BCI },
    { LD, DEI, A },
    { LD, A, DEI },
    { LD, ADDR, HL },
    { LD, HL, ADDR },
    { LD, ADDR, A },
    { LD, A, ADDR },

    // 3y1: Y_POP_MISC (offset 24): pops and miscellaneous instructions
    { POP, BC, _ },
    { RET, _, _ },
    { POP, DE, _ },
    { EXX, _, _ },
    { POP, HL, _ },
    { JP, HLI, _ },
    { POP, AF, _ },
    { LD, SP_, HL },

    // 3y3: Y_MISC (offset 32): miscellaneous instructions
    { JP, WORD, _ },
    { _, _, _ }, // CB prefix
    { OUT, PORT, A },
    { IN, A, PORT },
    { EX, SPI, HL },
    { EX, DE, NSHL },
    { DI, _, _ },
    { EI, _, _ },

    // 3y5: Y_PUSH_CALL (offset 40): push and call instructions
    { PUSH, BC, _ },
    { CALL, WORD, _ },
    { PUSH, DE, _ },
    { _, _, _ }, // DD prefix
    { PUSH, HL, _ },
    { _, _, _ }, // ED prefix
    { PUSH, AF, _ },
    { _, _, _ }, // FD prefix

    // ED-1y3: Y_LD16_IMM (offset 48): ED-prefixed extended 16-bit LD
    { LD, ADDR, BC },
    { LD, BC, ADDR },
    { LD, ADDR, DE },
    { LD, DE, ADDR },
    { LD, ADDR, HL },
    { LD, HL, ADDR },
    { LD, ADDR, SP_ },
    { LD, SP_, ADDR },

    // ED-1y7: Y_LD_IR_ROT (offset 56): ld, i/r, a and rrd/rld instructions
    { LD, I, A },
    { LD, R, A },
    { LD, A, I },
    { LD, A, R },
    { RRD, _, _ },
    { RLD, _, _ },
    { NOP, _, _ },
    { NOP, _, _ }
};

//////////////// X-based LUTs ////////////////

const instr_t instr_unprefixed[] PROGMEM = {
    { Z_0YZ, _, _ },
    { LD, Y_REG8, Z_REG8 },
    { Y_ALU, Z_REG8, _ },
    { Z_3YZ, _, _ }
};

const instr_t instr_ed_prefixed[] PROGMEM = {
    { NONI, _, _ },
    { Z_ED_1YZ, _, _ },
    { YZ_BLOCK, _, _ },
    { NONI, _, _ }
};

// cb-prefixed opcodes
const instr_t instr_cb_prefixed[] PROGMEM = {
    { Y_ROT, Z_REG8, _ },
    { BIT, Y_DIGIT, Z_REG8 },
    { RES, Y_DIGIT, Z_REG8 },
    { SET, Y_DIGIT, Z_REG8 }
};

//////////////// general functions ////////////////

// instructor lookup from specified table and index
instr_t i_lookup(const instr_t* tab, uint8_t n)
{
    instr_t i;
    i.op = pgm_read_byte(&tab[n].op);
    i.p1 = pgm_read_byte(&tab[n].p1);
    i.p2 = pgm_read_byte(&tab[n].p2);
    return i;
}

// x-based lookup for specified prefix and opcode
instr_t decode(uint8_t prefix, uint8_t opcode)
{
    const instr_t* tab;
    instr_t i = { HALT_, _, _ };
    switch (prefix) {
    case 0xCB:
        tab = instr_cb_prefixed;
        break;
    case 0xED:
        tab = instr_ed_prefixed;
        break;
    default:
        if (opcode == 0x76)
            return i;
        tab = instr_unprefixed;
    }

    uint8_t x = opcode >> 6;
    uint8_t y = (opcode >> 3) & 7;
    uint8_t z = opcode & 7;

    uint8_t offset;

    // x-based lookup
    i = i_lookup(tab, x);

    // z-based lookup
    if (TOKTYPE(i.op) == Z_INSTR) {
        offset = (i.op - Z_INSTR) * 8 + z;
        i = i_lookup(z_instrs, offset);
    }

    // y-based lookup
    if (TOKTYPE(i.op) == Y_INSTR) {
        offset = (i.op - Y_INSTR) * 8 + y;
        return i_lookup(y_instrs, offset);
    }

    // Common path for all non-Y_INSTR results: resolve tokens using t_lookup
    i.op = t_lookup(i.op, y, z);
    i.p1 = t_lookup(i.p1, y, z);
    i.p2 = t_lookup(i.p2, y, z);
    return i;
}