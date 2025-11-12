#ifndef DECODE_H
#define DECODE_H
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
 * @file decode.h Z80 disassembler instruction decoding
 */

#include <stdint.h>

// constant starting values for different classes of tokens
enum {
    Y_TOKEN = 0000,   // y-indexed token
    O_TOKEN = 0010,   // general token
    Y_INSTR = 0020,   // instruction lookup using Y
    Z_INSTR = 0030,   // instruction lookup using Z
    IMMEDIATE = 0040, // immediate operands
    LITERALS1 = 0100, // 1-character literals
    LITERALS4 = 0140, // 4-character literals
    LITERALS2 = 0200, // 2-character literals
    LITERALS3 = 0300, // 3-character literals
};

// token lookup tables
enum {
    Y_REG8 = Y_TOKEN, // 8-bit registers using y field
    Y_CC,             // condition codes
    Y_RST,            // rst parameters
    Y_IM,             // interrupt modes
    Y_ALU,            // alu operations
    Y_ROT,            // bit rotate operations
    Y_FLAG,           // operations on accumulator/flags
    P_REG16,          // 16-bit registers selected by p field
};

// other tokens
enum {
    Z_REG8 = O_TOKEN, // 8-bit registers selected by z field
    Y_DIGIT,          // digits 0-7 selected by y field
    Q_INC_DEC,        // inc/dec selected by q field
    Q_SBC_ADC,        // sbc/adc selected by q field
    Y_RETI_RETN,      // reti/retn selected by y field
    YZ_BLOCK          // block operations selected by y and z fields
};

// instruction lookups selected by y
enum {
    Y_JR_MISC = Y_INSTR, // relative jumps and miscellaneous
    Y_LD_ADD_16,         // 16-bit ld/add
    Y_LD_IND,            // 8/16-bit load indirect
    Y_POP_MISC,          // pop and miscellaneous
    Y_MISC,              // miscellaneous
    Y_PUSH_CALL,         // push and call
    Y_LD16_IMM,          // 16-bit ld rp16,imm / ld imm,rp16
    Y_LD_IR_ROT          // ld a, i/r and rld/rrd
};

// instruction lookups selcted by z
enum {
    Z_0YZ = Z_INSTR,
    Z_3YZ,
    Z_ED_1YZ
};

// immediate modes
enum {
    PORT = IMMEDIATE, // 8-bit port address
    ADDR,             // 16-bit memory address
    IXD,              // ix + 8-bit displacement
    IYD,              // iy + 8-bit displacement
    PAREN,            // indirect must come before; direct must come after
    DISP,             // 8-bit signed displacement
    BYTE,             // byte
    WORD              // word
};

// clang-format off

// 1-character literals
enum {
    // numbers
    N0 = LITERALS1, N1, N2, N3, N4, N5, N6, N7,
    // registers
    B, C, D, E, H, L, A, I, R,
    // condition codes
    Z, P, M,
    // blank
    _
};

// 2-character literals
enum {
    // instructions
    CP = LITERALS2, DI, EI, EX, IM, IN, JP, JR, LD, OR, RL, RR, 
    // registers
    BC, DE, HL, NSHL, SP_, AF, IX, IY, IXI, IYI,
    // condition codes
    NZ, NC, PO, PE,
    // prefixes
    CB, DD, ED, FD
};

// 3-character literals
enum {
    // instructions
    ADC_ = LITERALS3, ADD, AND, BIT, CCF, CPD, CPI, CPL, 
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

// 4-character literals
enum {
    // instructions
    CALL = LITERALS4, CPDR, CPIR, DJNZ, HALT_, INDR, INIR, LDDR, LDIR,
    OTDR, OTIR, OUTD, OUTI, PUSH, RETI, RETN, RLCA, RRCA,
    // undocumented instructions
    NOP8, NONI,
    // registers
    BCI, DEI, HLI, SPI
};

#define TOKTYPE(t) ((t) & 0370)               // token type
#define TOKORD(t) ((t) & 7)                   // token ordinal
#define LOOKUP(tab, i) pgm_read_byte(&tab[i]) // look up byte from progmem

// Instruction with one operation and two parameters
typedef struct
{
    uint8_t op; // operation
    uint8_t p1; // first parameter
    uint8_t p2; // second parameter
} instr_t;

instr_t decode(uint8_t prefix, uint8_t opcode);

#endif