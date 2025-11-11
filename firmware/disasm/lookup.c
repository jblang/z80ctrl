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

//////////////// general functions ////////////////

// instruction constructor
instr_t instr(uint8_t op, uint8_t p1, uint8_t p2)
{
    instr_t i;
    i.op = op;
    i.p1 = p1;
    i.p2 = p2;
    return i;
}

// instructor lookup from specified table and index
instr_t i_lookup(const instr_t* tab, uint8_t n)
{
    instr_t i;
    i.op = pgm_read_byte(&tab[n].op);
    i.p1 = pgm_read_byte(&tab[n].p1);
    i.p2 = pgm_read_byte(&tab[n].p2);
    return i;
}

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
    uint8_t index;
    uint8_t offset = tok;

    if (tok == P_REG16) {
        // P_REG16 is now in Y_TOKEN enum at position 7
        index = p;
    } else if (tok == Z_REG8) {
        // Z_REG8 uses same table as Y_REG8 (both are 8-bit regs)
        offset = Y_REG8;
        index = z;
    } else if (TOKTYPE(tok) == Y_TOKEN) {
        index = y;
    } else {
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
        default:
            // Literal tokens pass through
            return tok;
        }
    }
    return LOOKUP(tokens, offset * 8 + index);
}

//////////////// Y-based LUTs ////////////////

// 0y0: relative jumps and miscellaneous
const instr_t instr_jr_misc[] PROGMEM = {
    { NOP, _, _ },
    { EX, AF, AFP },
    { DJNZ, DISP, _ },
    { JR, DISP, _ },
    { JR, NZ, DISP },
    { JR, Z, DISP },
    { JR, NC, DISP },
    { JR, C, DISP }
};

// 0y1: 16-bit immediate load / ADD HL operations
const instr_t instr_ld_add_16_imm[] PROGMEM = {
    { LD, BC, WORD },
    { ADD, HL, BC },
    { LD, DE, WORD },
    { ADD, HL, DE },
    { LD, HL, WORD },
    { ADD, HL, HL },
    { LD, SP_, WORD },
    { ADD, HL, SP_ }
};

const instr_t instr_ld16_ind[] PROGMEM = {
    { LD, BCI, A },
    { LD, A, BCI },
    { LD, DEI, A },
    { LD, A, DEI },
    { LD, ADDR, HL },
    { LD, HL, ADDR },
    { LD, ADDR, A },
    { LD, A, ADDR }
};

// 3y2: pops and miscellaneous instructions
const instr_t instr_pop_misc[] PROGMEM = {
    { POP, BC, _ },
    { RET, _, _ },
    { POP, DE, _ },
    { EXX, _, _ },
    { POP, HL, _ },
    { JP, HLI, _ },
    { POP, AF, _ },
    { LD, SP_, HL }
};

// 3y3: miscellaneous instructions
const instr_t instr_misc[] PROGMEM = {
    { JP, WORD, _ },
    { _, _, _ }, // CB prefix
    { OUT, PORT, A },
    { IN, A, PORT },
    { EX, SPI, HL },
    { EX, DE, NSHL },
    { DI, _, _ },
    { EI, _, _ }
};

// 3y5: push and call instructions
const instr_t instr_push_call[] PROGMEM = {
    { PUSH, BC, _ },
    { CALL, WORD, _ },
    { PUSH, DE, _ },
    { _, _, _ }, // DD prefix
    { PUSH, HL, _ },
    { _, _, _ }, // ED prefix
    { PUSH, AF, _ },
    { _, _, _ } // FD prefix
};

// 1y7: ld, i/r, a and rrd/rld instructions
const instr_t instr_ld_ir_rot[] PROGMEM = {
    { LD, I, A },
    { LD, R, A },
    { LD, A, I },
    { LD, A, R },
    { RRD, _, _ },
    { RLD, _, _ },
    { NOP, _, _ },
    { NOP, _, _ }
};

// y-based lookup for specified token and y-value
instr_t y_lookup(uint8_t tok, uint8_t y)
{
    const instr_t* tab;
    switch (tok) {
    case Y_JR_MISC:
        tab = instr_jr_misc;
        break;
    case Y_LD_ADD_16:
        tab = instr_ld_add_16_imm;
        break;
    case Y_LD_IND:
        tab = instr_ld16_ind;
        break;
    case Y_POP_MISC:
        tab = instr_pop_misc;
        break;
    case Y_MISC:
        tab = instr_misc;
        break;
    case Y_PUSH_CALL:
        tab = instr_push_call;
        break;
    case Y_LD_IR_ROT:
        tab = instr_ld_ir_rot;
        break;
    case Y_LD16_IMM: {
        // ED-prefixed extended 16-bit LD: LD (nn), rr or LD rr, (nn)
        // q=0: LD (nn), rr; q=1: LD rr, (nn)
        // ADDR < PAREN, so it will automatically get parentheses
        instr_t i;
        if (y & 1) {
            i = instr(LD, P_REG16, ADDR); // LD rr, (nn)
        } else {
            i = instr(LD, ADDR, P_REG16); // LD (nn), rr
        }
        // Resolve P_REG16 token using y value
        i.p1 = t_lookup(i.p1, y, 0);
        i.p2 = t_lookup(i.p2, y, 0);
        return i;
    }
    }
    return i_lookup(tab, y);
}

//////////////// Z-based LUTs ////////////////

// 0yz: unprefixed instructions from 000-077
const instr_t instr_0yz[] PROGMEM = {
    { Y_JR_MISC, _, _ },
    { Y_LD_ADD_16, _, _ },
    { Y_LD_IND, _, _ },
    { Q_INC_DEC, P_REG16, _ },
    { INC, Y_REG8, _ },
    { DEC, Y_REG8, _ },
    { LD, Y_REG8, BYTE },
    { Y_FLAG, _, _ }
};

// 3yz: unprefixed instructions from 300-377
const instr_t instr_3yz[] PROGMEM = {
    { RET, Y_CC, _ },
    { Y_POP_MISC, _, _ },
    { JP, Y_CC, WORD },
    { Y_MISC, _, _ },
    { CALL, Y_CC, WORD },
    { Y_PUSH_CALL, _, _ },
    { Y_ALU, A, BYTE },
    { RST, Y_RST, _ }
};

// ed-1yz: ED-prefixed instructions from 100-177
const instr_t instr_ed_1yz[] PROGMEM = {
    { IN, Y_REG8, CI },
    { OUT, CI, Y_REG8 },
    { Q_SBC_ADC, HL, P_REG16 },
    { Y_LD16_IMM, _, _ },
    { NEG, _, _ },
    { Y_RETI_RETN, _, _ },
    { IM, Y_IM, _ },
    { Y_LD_IR_ROT, _, _ }
};

// z-based lookup for specified token and z-value
instr_t z_lookup(uint8_t tok, uint8_t y, uint8_t z)
{
    const instr_t* tab;
    switch (tok) {
    case Z_0YZ:
        tab = instr_0yz;
        break;
    case Z_3YZ:
        tab = instr_3yz;
        break;
    case Z_ED_1YZ:
        tab = instr_ed_1yz;
        break;
    }
    instr_t i = i_lookup(tab, z);
    if (TOKTYPE(i.op) == Y_INSTR) {
        return y_lookup(i.op, y);
    } else {
        i.op = t_lookup(i.op, y, z);
        i.p1 = t_lookup(i.p1, y, z);
        i.p2 = t_lookup(i.p2, y, z);
        return i;
    }
}

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

// x-based lookup for specified prefix and opcode
instr_t decode(uint8_t prefix, uint8_t opcode)
{
    uint8_t x = opcode >> 6;
    uint8_t y = (opcode >> 3) & 7;
    uint8_t z = opcode & 7;
    const instr_t* tab;

    switch (prefix) {
    case 0xCB:
        tab = instr_cb_prefixed;
        break;
    case 0xED:
        tab = instr_ed_prefixed;
        break;
    default:
        if (opcode == 0x76)
            return instr(HALT_, _, _);
        tab = instr_unprefixed;
    }

    instr_t i = i_lookup(tab, x);
    switch (TOKTYPE(i.op)) {
    case Z_INSTR:
        return z_lookup(i.op, y, z);
    case Y_INSTR:
        return y_lookup(i.op, y);
    default:
        i.op = t_lookup(i.op, y, z);
        i.p1 = t_lookup(i.p1, y, z);
        i.p2 = t_lookup(i.p2, y, z);
        return i;
    }
}