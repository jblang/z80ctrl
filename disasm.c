#include "disasm.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// reference: http://www.z80.info/decoding.htm

// 8-bit registers
char *registers[] = {"B", "C", "D", "E", "H", "L", "(HL)", "A", "IXH", "IXL", "IYH", "IYL"};
enum {B, C, D, E, H, L, HLI, A, IXH, IXL, IYH, IYL};

// 16-bit register pairs
char *register_pairs[] = {"BC", "DE", "HL", "SP", "AF", "IX", "IY"};
enum {BC, DE, HL, SP, AF, IX, IY};

// condition codes
char *conditions[] = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};

// arithmetic/logic operations
char *alu_ops[] = {"ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP "};

// rotation/shift operations
char *rot_ops[] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL"};

// interrupt modes
char *int_modes[] = {"0", "0", "1", "2"};

// block instructions
char *block_ops[] = {
    "LDI", "LDD", "LDIR", "LDDR", 
    "CPI", "CPD", "CPIR", "CPDR", 
    "INI", "IND", "INIR", "INDR", 
    "OUTI", "OUTD", "OTIR", "OTDR"
};

// indirect load ops
char *ld_ops[] = {"LD (BC),A", "LD A,(BC)", "LD (DE),A", "LD A,(DE)"};

// accumulator and flag ops for x = 0, z = 7
char *af_ops[] = {"RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF"};

// miscellaneous 0xED operations
char *misc_ops[] = {"LD I,A", "LD R,A", "LD A,I", "LD A,R", "RRD", "RLD", "NOP", "NOP"};

char *bit_ops[] = {"BIT", "RES", "SET"};

uint8_t disasm(uint16_t addr, uint8_t (*input)(), char *output)
{
    uint8_t opcode = 0;
    uint16_t prefix = 0;
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

    char *rp, *hli, *ry, *ryi, *rz, *rzi;

    // choose registers based on index mode and y/z/p opcode fields
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

    if (prefix == 0xCB) {
        if (x == 0) {
            if (im == HL) {
                sprintf(output, "%s %s", rot_ops[y], rz);
            } else {
                sprintf(output, "%s (%s+%02XH)", rot_ops[y], hli, displ);
            }
        } else {
            if (im == HL) {
                sprintf(output, "%s %X,%s", bit_ops[x-1], y, rz);
            } else {
                sprintf(output, "%s %X,(%s+%02XH)", bit_ops[x-1], y, hli, displ);
            }
        }
    } else if (prefix == 0xED) {
        if (x == 1) {
            if (z == 0) {
                sprintf(output, (y == 6) ? "IN (C)" : "IN %s,(C)", ry);
            } else if (z == 1) {
                sprintf(output, (y == 6) ? "OUT (C)" : "OUT (C),%s", ry);
            } else if (z == 2) {
                sprintf(output, (q == 0) ? "SBC HL,%s" : "ADC HL,%s", rp);
            } else if (z == 3) {
                operand = input() | (input() << 8);
                if (q == 0) {
                    sprintf(output, "LD (%04XH),%s", operand, rp);
                } else {
                    sprintf(output, "LD %s,(%04XH)", rp, operand);
                }
            } else if (z == 4) {
                strcpy(output, "NEG");
            } else if (z == 5) {
                strcpy(output, (y == 1) ? "RETI" : "RETN");
            } else if (z == 6) {
                sprintf(output, "IM %s", int_modes[y&0x3]);
            } else if (z == 7) {
                strcpy(output, misc_ops[y]);
            }
        } else if (x == 2 && z <= 3 && y >= 4) {
                strcpy(output, block_ops[zy]);
        } else {
            sprintf(output, "?");
        }
    } else {
        // un-prefixed opcodes
        if (x == 0) {
            if (z == 0) {
                if (y == 0) {
                    strcpy(output, "NOP");
                } else if (y == 1) {
                    strcpy(output, "EX AF,AF'");
                } else {
                    operand = input();
                    if (y == 2) {
                        sprintf(output, "DJNZ %d", (int8_t)operand);
                    } else if (y == 3) {
                        sprintf(output, "JR %d", (int8_t)operand);
                    } else {
                        sprintf(output, "JR %s,%d", conditions[y-4], (int8_t)operand);
                    }
                }
            } else if (z == 1) {
                if (q == 0) {
                    operand = input() | (input() << 8);
                    sprintf(output, "LD %s,%04XH", rp, operand);
                } else {
                    sprintf(output, "ADD %s,%s", hli, rp);
                }
            } else if (z == 2) {
                if (y < 4) {
                    strcpy(output, ld_ops[y]);
                } else {
                    operand = input() | (input() << 8);
                    if (p == 3) {
                        hli = "A";
                    }
                    if (q == 0) {
                        sprintf(output, "LD (0%04XH),%s", operand, hli); 
                    } else {
                        sprintf(output, "LD %s,(0%04XH)", hli, operand);
                    }
                }
            } else if (z == 3) {
                sprintf(output, (q == 0) ? "INC %s" : "DEC %s", rp);
            } else if (z == 4) {
                if (y == HLI && im != HL) {
                    displ = input();
                    sprintf(output, "INC (%s+%02XH)", hli, displ);
                } else {
                    sprintf(output, "INC %s", ry);
                }
            } else if (z == 5) {
                if (y == HLI && im != HL) {
                    displ = input();
                    sprintf(output, "DEC (%s+%02XH)", hli, displ);
                } else {
                    sprintf(output, "DEC %s", ry);
                }
            } else if (z == 6) {
                if (y == HLI && im != HL) {
                    displ = input();
                    operand = input();
                    sprintf(output, "LD (%s+%02XH),%02XH", hli, displ, operand);
                } else {
                    operand = input();
                    sprintf(output, "LD %s,%02XH", ry, operand);
                }
            } else if (z == 7) {
                strcpy(output, af_ops[y]);
            }
        } else if (x == 1) {
            if (z == HLI && y == HLI) {
                strcpy(output, "HALT");
            } else if ((y == HLI || z == HLI) && im != HL) {
                displ = input();
                if (y == HLI) {
                    sprintf(output, "LD (%s+%02XH),%s", hli, displ, rz);
                } else {
                    sprintf(output, "LD %s,(%s+%02XH)", ry, hli, displ);
                }
            } else {
                sprintf(output, "LD %s,%s", ry, rz);
            }
        } else if (x == 2) {
            if (z == 6 && im != HL) {
                displ = input();
                sprintf(output, "%s(%s+%02XH)", alu_ops[y], hli, displ);
            } else {
                sprintf(output, "%s%s", alu_ops[y], rz);
            }
        } else if (x == 3) {
            if (z == 0) {
                sprintf(output, "RET %s", conditions[y]);
            } else if (z == 1) {
                if (q == 0) {
                    sprintf(output, "POP %s", p < SP ? rp : register_pairs[AF]);
                } else if (p == 0) {
                    strcpy(output, "RET");
                } else if (p == 1) {
                    strcpy(output, "EXX");
                } else if (p == 2) {
                    sprintf(output, "JP (%s)", hli);
                } else if (p == 3) {
                    sprintf(output, "LD SP,%s", hli);
                }
            } else if (z == 2) {
                operand = input() | (input() << 8);
                sprintf(output, "JP %s,%04XH", conditions[y], operand);
            } else if (z == 3) {
                if (y == 0) {
                    operand = input() | (input() << 8);                                    
                    sprintf(output, "JP %04XH", operand);
                } else if (y == 1) {
                    strcpy(output, "?"); // CB prefix
                } else if (y == 2) {
                    operand = input();
                    sprintf(output, "OUT (%02XH),A", operand);
                } else if (y == 3) {
                    operand = input();
                    sprintf(output, "IN A,(%02XH)", operand);
                } else if (y == 4) {
                    sprintf(output, "EX (SP),%s", hli);
                } else if (y == 5) {
                    strcpy(output, "EX DE,HL");
                } else if (y == 6) {
                    strcpy(output, "DI");
                } else if (y == 7) {
                    strcpy(output, "EI");
                }
            } else if (z == 4) {
                operand = input() | (input() << 8);
                sprintf(output, "CALL %s,%04XH", conditions[y], operand);
            } else if (z == 5) {
                if (q == 0)
                    sprintf(output, "PUSH %s", p < SP ? rp : register_pairs[AF]);
                else if (p == 0) {
                    operand = input() | (input() << 8);
                    sprintf(output, "CALL %04XH", operand);
                } 
            } else if (z == 6) {
                operand = input();
                sprintf(output, "%s%02XH", alu_ops[y], operand);
            } else if (z == 7) {
                sprintf(output, "RST %02XH", y*8);
            }
        }
    }

    //printf("%s %02X %03o %04X\t", register_pairs[im], prefix, opcode, operand);
}