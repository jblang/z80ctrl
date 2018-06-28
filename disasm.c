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

/**
 *  8-bit registers
 */
const char *registers[] = {"B", "C", "D", "E", "H", "L", "(HL)", "A", "IXH", "IXL", "IYH", "IYL"};
enum {B, C, D, E, H, L, HLI, A, IXH, IXL, IYH, IYL};

/**
 *  16-bit register pairs
 */
const char *register_pairs[] = {"BC", "DE", "HL", "SP", "AF", "IX", "IY"};
enum {BC, DE, HL, _SP, AF, IX, IY};

/**
 * condition codes
 */
const char *conditions[] = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};

/**
 * arithmetic/logic operations
 */
const char *alu_ops[] = {"ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP "};

/**
 * rotation/shift operations
 */
const char *rot_ops[] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL"};

/**
 * interrupt modes
 */
const char *int_modes[] = {"0", "0", "1", "2"};

/**
 * block instructions
 */
const char *block_ops[] = {
    "LDI", "LDD", "LDIR", "LDDR", 
    "CPI", "CPD", "CPIR", "CPDR", 
    "INI", "IND", "INIR", "INDR", 
    "OUTI", "OUTD", "OTIR", "OTDR"
};

/**
 * indirect load ops
 */
const char *ld_ops[] = {"LD (BC),A", "LD A,(BC)", "LD (DE),A", "LD A,(DE)"};

/**
 * accumulator and flag ops for x = 0, z = 7
 */
const char *af_ops[] = {"RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF"};

/**
 * miscellaneous 0xED operations
 */
const char *misc_ops[] = {"LD I,A", "LD R,A", "LD A,I", "LD A,R", "RRD", "RLD", "NOP", "NOP"};

const char *bit_ops[] = {"BIT", "RES", "SET"};

/**
 * disassemble a single instruction
 */
uint8_t disasm(uint8_t (*input)(), char *output)
{
    uint8_t opcode = 0;
    uint8_t prefix = 0;
    uint8_t displ = 0;
    uint16_t operand = 0;
    uint8_t im = HL;

    // Consume any number of 0xDD and 0xFD prefix bytes
    // and set index mode according to last one encountered.
    for (;;) {
            opcode = input();
            if (opcode == 0xDD)
                im = IX;
            else if (opcode == 0xFD)
                im = IY;
            else
                break;
    }

    if (opcode == 0xED) {
            prefix = 0xED;
            im = HL;            // Index mode for 0xED prefix is always HL
            opcode = input();
    } if (opcode == 0xCB) {
        prefix = 0xCB;
        if (im != HL)
                displ = input();
        opcode = input();
    } 

    // bit slice the opcode: xxyyyzzz / xxppqzzz
    uint8_t x = (opcode & 0300) >> 6;       // x = opcode[7:6]
    uint8_t y = (opcode & 0070) >> 3;       // y = opcode[5:3]
    uint8_t z = (opcode & 0007);            // z = opcode[2:0]
    uint8_t p = (opcode & 0060) >> 4;       // p = opcode[5:4]
    uint8_t q = (opcode & 0010) >> 3;       // q = opcode[3]
    uint8_t zy = ((z & 3) << 2) | (y & 3);  // zy = {opcode[1:0], opcode[4:3]}

    // choose registers based on index mode and y/z/p opcode fields
    const char *rp, *hli, *ry, *ryi, *rz, *rzi;
    rp = register_pairs[p == HL ? im : p];
    ry = registers[y];
    rz = registers[z];
    if (im == IX) {
        hli = register_pairs[IX];
        ryi = registers[y == H ? IXH : y == L ? IXL : y];
        rzi = registers[z == H ? IXH : z == L ? IXL : z];
    } else if (im == IY) {
        hli = register_pairs[IY];
        ryi = registers[y == H ? IYH : y == L ? IYL : y];
        rzi = registers[z == H ? IYH : z == L ? IYL : z];
    } else {
        hli = register_pairs[HL];
        ryi = ry;
        rzi = rz;
    }

    // Big ugly nested if tree to decode opcode
    if (prefix == 0xCB) {
        if (x == 0) {
            // Roll/shift register or memory location
            if (im == HL) {
                sprintf_P(output, PSTR("%s %s"), rot_ops[y], rz);
            } else {
                sprintf_P(output, PSTR("%s (%s+%02XH)"), rot_ops[y], hli, displ);
            }
        } else {
            // Bit operations (test reset, set)
            if (im == HL) {
                sprintf_P(output, PSTR("%s %X,%s"), bit_ops[x-1], y, rz);
            } else {
                sprintf_P(output, PSTR("%s %X,(%s+%02XH)"), bit_ops[x-1], y, hli, displ);
            }
        }
    } else if (prefix == 0xED) {
        if (x == 1) {
            if (z == 0) {
                // Input from port
                sprintf_P(output, (y == 6) ? PSTR("IN (C)") : PSTR("IN %s,(C)"), ry);
            } else if (z == 1) {
                // Output to port
                sprintf_P(output, (y == 6) ? PSTR("OUT (C)") : PSTR("OUT (C),%s"), ry);
            } else if (z == 2) {
                // 16-bit add/subtract with carry
                sprintf_P(output, (q == 0) ? PSTR("SBC HL,%s") : PSTR("ADC HL,%s"), rp);
            } else if (z == 3) {
                // Retrieve/store register pair from/to immediate address
                operand = input() | (input() << 8);
                if (q == 0) {
                    sprintf_P(output, PSTR("LD (%04XH),%s"), operand, rp);
                } else {
                    sprintf_P(output, PSTR("LD %s,(%04XH)"), rp, operand);
                }
            } else if (z == 4) {
                // Negate accumulator
                strcpy_P(output, PSTR("NEG"));
            } else if (z == 5) {
                // Return from interrupt
                strcpy_P(output, (y == 1) ? PSTR("RETI") : PSTR("RETN"));
            } else if (z == 6) {
                // Set interrupt mode
                sprintf_P(output, PSTR("IM %s"), int_modes[y&0x3]);
            } else if (z == 7) {
                // Assorted ops
                strcpy(output, misc_ops[y]);
            }
        } else if (x == 2 && z <= 3 && y >= 4) {
                strcpy(output, block_ops[zy]);
        } else {
            sprintf_P(output, PSTR("?"));
        }
    } else {
        // un-prefixed opcodes
        if (x == 0) {
            if (z == 0) {
                // Relative jumps and assorted ops
                if (y == 0) {
                    strcpy_P(output, PSTR("NOP"));
                } else if (y == 1) {
                    strcpy_P(output, PSTR("EX AF,AF'"));
                } else {
                    operand = input();
                    if (y == 2) {
                        sprintf_P(output, PSTR("DJNZ %d"), (int8_t)operand);
                    } else if (y == 3) {
                        sprintf_P(output, PSTR("JR %d"), (int8_t)operand);
                    } else {
                        sprintf_P(output, PSTR("JR %s,%d"), conditions[y-4], (int8_t)operand);
                    }
                }
            } else if (z == 1) {
                // 16-bit load immediate/add
                if (q == 0) {
                    operand = input() | (input() << 8);
                    sprintf_P(output, PSTR("LD %s,%04XH"), rp, operand);
                } else {
                    sprintf_P(output, PSTR("ADD %s,%s"), hli, rp);
                }
            } else if (z == 2) {
                // Indirect loading
                if (y < 4) {
                    strcpy(output, ld_ops[y]);
                } else {
                    operand = input() | (input() << 8);
                    if (p == 3) {
                        hli = "A";
                    }
                    if (q == 0) {
                        sprintf_P(output, PSTR("LD (0%04XH),%s"), operand, hli); 
                    } else {
                        sprintf_P(output, PSTR("LD %s,(0%04XH)"), hli, operand);
                    }
                }
            } else if (z == 3) {
                // 16-bit increment or decrement
                sprintf_P(output, (q == 0) ? PSTR("INC %s") : PSTR("DEC %s"), rp);
            } else if (z == 4) {
                // 8-bit increment
                if (y == HLI && im != HL) {
                    displ = input();
                    sprintf_P(output, PSTR("INC (%s+%02XH)"), hli, displ);
                } else {
                    sprintf_P(output, PSTR("INC %s"), ry);
                }
            } else if (z == 5) {
                // 8-bit decrement
                if (y == HLI && im != HL) {
                    displ = input();
                    sprintf_P(output, PSTR("DEC (%s+%02XH)"), hli, displ);
                } else {
                    sprintf_P(output, PSTR("DEC %s"), ry);
                }
            } else if (z == 6) {
                // 8-bit load immediate
                if (y == HLI && im != HL) {
                    displ = input();
                    operand = input();
                    sprintf_P(output, PSTR("LD (%s+%02XH),%02XH"), hli, displ, operand);
                } else {
                    operand = input();
                    sprintf_P(output, PSTR("LD %s,%02XH"), ry, operand);
                }
            } else if (z == 7) {
                // Assorted operations on accumulator/flags
                strcpy(output, af_ops[y]);
            }
        } else if (x == 1) {
            if (z == HLI && y == HLI) {
                // Exception: HALT replaces LD (HL),(HL)
                strcpy_P(output, PSTR("HALT"));
            } else if ((y == HLI || z == HLI) && im != HL) {
                // 8-bit loading
                displ = input();
                if (y == HLI) {
                    sprintf_P(output, PSTR("LD (%s+%02XH),%s"), hli, displ, rz);
                } else {
                    sprintf_P(output, PSTR("LD %s,(%s+%02XH)"), ry, hli, displ);
                }
            } else {
                sprintf_P(output, PSTR("LD %s,%s"), ry, rz);
            }
        } else if (x == 2) {
            // ALU operation on accumulator and register/memory location
            if (z == 6 && im != HL) {
                displ = input();
                sprintf_P(output, PSTR("%s(%s+%02XH)"), alu_ops[y], hli, displ);
            } else {
                sprintf_P(output, PSTR("%s%s"), alu_ops[y], rz);
            }
        } else if (x == 3) {
            if (z == 0) {
                // Conditional return
                sprintf_P(output, PSTR("RET %s"), conditions[y]);
            } else if (z == 1) {
                // POP & various ops
                if (q == 0) {
                    sprintf_P(output, PSTR("POP %s"), p < _SP ? rp : register_pairs[AF]);
                } else if (p == 0) {
                    strcpy_P(output, PSTR("RET"));
                } else if (p == 1) {
                    strcpy_P(output, PSTR("EXX"));
                } else if (p == 2) {
                    sprintf_P(output, PSTR("JP (%s)"), hli);
                } else if (p == 3) {
                    sprintf_P(output, PSTR("LD _SP,%s"), hli);
                }
            } else if (z == 2) {
                // Conditional jump
                operand = input() | (input() << 8);
                sprintf_P(output, PSTR("JP %s,%04XH"), conditions[y], operand);
            } else if (z == 3) {
                // Assorted operations
                if (y == 0) {
                    operand = input() | (input() << 8);                                    
                    sprintf_P(output, PSTR("JP %04XH"), operand);
                } else if (y == 1) {
                    strcpy_P(output, PSTR("?")); // CB prefix
                } else if (y == 2) {
                    operand = input();
                    sprintf_P(output, PSTR("OUT (%02XH),A"), operand);
                } else if (y == 3) {
                    operand = input();
                    sprintf_P(output, PSTR("IN A,(%02XH)"), operand);
                } else if (y == 4) {
                    sprintf_P(output, PSTR("EX (_SP),%s"), hli);
                } else if (y == 5) {
                    strcpy_P(output, PSTR("EX DE,HL"));
                } else if (y == 6) {
                    strcpy_P(output, PSTR("DI"));
                } else if (y == 7) {
                    strcpy_P(output, PSTR("EI"));
                }
            } else if (z == 4) {
                // Conditional call
                operand = input() | (input() << 8);
                sprintf_P(output, PSTR("CALL %s,%04XH"), conditions[y], operand);
            } else if (z == 5) {
                // PUSH & various ops
                if (q == 0)
                    sprintf_P(output, PSTR("PUSH %s"), p < _SP ? rp : register_pairs[AF]);
                else if (p == 0) {
                    operand = input() | (input() << 8);
                    sprintf_P(output, PSTR("CALL %04XH"), operand);
                } 
            } else if (z == 6) {
                // ALU operation on accumulator and immediate operand
                operand = input();
                sprintf_P(output, PSTR("%s%02XH"), alu_ops[y], operand);
            } else if (z == 7) {
                // Restart
                sprintf_P(output, PSTR("RST %02XH"), y*8);
            }
        }
    }

    //printf_P(PSTR("%s %02X %03o %04X\t"), register_pairs[im], prefix, opcode, operand);
}

uint8_t disasm_index = 0;   /**< index of next byte within 256 byte buffer */
uint32_t disasm_addr = 0;   /**< address of next chunk to read from external RAM */
uint8_t *disasm_buf;        /**< pointer to disassembly buffer */

uint8_t instr_bytes[8];     /**< bytes contained in the current instruction */
uint8_t instr_length = 0;   /**< length of the current construction */

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
        printf_P(PSTR("%05lX  "), disasm_addr + base_addr);
        disasm(disasm_next_byte, mnemonic);
        for (i = 0; i < instr_length; i++) {
            printf_P(PSTR("%02X "), instr_bytes[i]);
        }
        i = 5 - instr_length;
        while (i--)
            printf_P(PSTR("   "));
        printf_P(PSTR("  "));
        for (i = 0; i < instr_length; i++) {
            if (0x20 <= instr_bytes[i] && instr_bytes[i] <= 0x7e)
                printf_P(PSTR("%c"), instr_bytes[i]);
            else
                printf_P(PSTR("."));
        }
        i = 5 - instr_length;
        while (i--)
            printf_P(PSTR(" "));
        printf_P(PSTR("  %s\n"), mnemonic);            
    }
}