#include "disasm.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// reference: http://www.z80.info/decoding.htm

// 8-bit registers
char *r[] = {
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "(HL)",
    "A",
    "IXH",
    "IXL",
    "(IX+%02XH)",
    "IYH",
    "IYL",
    "(IY+%02XH)"
};

// 16-bit register pairs
char *rp[] = {
    "BC",
    "DE",
    "HL",
    "SP",
    "AF"
};

// condition codes
char *cc[] = {
    "NZ",
    "Z",
    "NC",
    "C",
    "PO",
    "PE",
    "P",
    "M"
};

// arithmetic/logic operations
char *alu[] = {
    "ADD A,%s",
    "ADC A,%s",
    "SUB A,%s",
    "SBC A,%s",
    "AND %s",
    "XOR %s",
    "OR %s",
    "CP %s"
};

// rotation/shift operations
char *rot[] = {
    "RLC",
    "RRC",
    "RL",
    "RR",
    "SLA",
    "SRA",
    "SLL",
    "SRL"
};

// interrupt modes
char *im[] = {
    "0",
    "0",
    "1",
    "2"
};

// block instructions
char *bli[] = {
    "LDI",
    "LDD",
    "LDIR",
    "LDDR",
    "CPI",
    "CPD",
    "CPIR",
    "CPDR",
    "INI",
    "IND"
    "INIR",
    "INDR",
    "OUTI",
    "OUTD",
    "OTIR",
    "OTDR"
};

// load ops for x = 0, z = 2
char *ldops[] = {
    "LD (BC),A",
    "LD A,(BC)",
    "LD (DE),A",
    "LD A,(DE)",
    "LD (%04XH),HL",
    "LD HL,(%04XH)",
    "LD (%04XH),A",
    "LD A,(%04XH)"        
};

// accumulator and flag ops for x = 0, z = 7
char *afops[] = {
    "RLCA",
    "RRCA",
    "RLA",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF"
};

// misc ops for x = 3, z = 1, q = 1
char *x3z1q1ops[] = {
    "RET",
    "EXX",
    "JP HL",
    "LD SP,HL"
};

// misc operations for x = 3, z = 3
char *exintops[] = {
    "EX (SP),HL",
    "EX DE,HL",
    "DI",
    "EI"
};

char *edx1z7ops[] = {
    "LD I,A",
    "LD R,A",
    "LD A,I",
    "LD A,R",
    "RRD",
    "RLD",
    "NOP",
    "NOP"
};

uint8_t disasm(uint16_t addr, uint8_t (*input)(), char *output)
{
    uint8_t opcode = 0;
    uint16_t prefix = 0;
    uint8_t displ = 0;
    uint8_t imm = 0;
    enum {HL, IX, IY} idxmode = HL;

    // Consume any number of leading 0xDD and 0xFD bytes
    // and set index mode according to last one encountered.
    for (;;) {
            opcode = input();
            if (opcode == 0xDD)
                    idxmode = IX;
            else if (opcode == 0xFD)
                    idxmode = IY;
            else {
                    // Encountering 0xED changes index mode back to default
                    if (opcode == 0xED)
                            idxmode = HL;
                    // Any byte other than 0xDD or 0xFD terminates the loop
                    break;
            }
    }

    // Select lookup table based on prefix byte
    // and fetch displacement if required
    if (opcode == 0xED) {
            prefix = 0xED;
            opcode = input();
    } else if (opcode == 0xCB) {
            if (idxmode != HL)
                    displ = input();
            prefix = 0xCB;
            opcode = input();
    } else if (idxmode != HL && (opcode & 0x40 == 4 || opcode & 0xc0 == 0x40))
            displ = input();

    // bit slice the opcode
    uint8_t x = (opcode & 0xc0) >> 6;       // x = opcode[7:6]
    uint8_t y = (opcode & 0x3a) >> 3;       // y = opcode[5:3]
    uint8_t z = (opcode & 0x07);            // z = opcode[2:0]
    uint8_t p = (opcode & 0x30) >> 4;       // p = opcode[5:4]
    uint8_t q = (opcode & 0x08) >> 3;       // q = opcode[3]
    uint8_t yz = ((y & 3) << 2) | (z & 3);  // yz = {opcode[1:0], opcode[4:3]}

    printf("%x %02x %02x [%X %X %X <%X:%X> %X] ", idxmode, prefix, opcode, x, z, y, p, q, yz);
    switch (prefix) {
        case 0xCB:
            switch (x) {
                case 0:
                    sprintf(output, "%s %s", rot[y], r[z]);
                    break;
                case 1:
                    sprintf(output, "BIT %X,%s", y, r[z]);
                    break;
                case 2:
                    sprintf(output, "RES %X,%s", y, r[z]);
                    break;
                case 3:
                    sprintf(output, "SET %X,%s", y, r[z]);
                    break;                
            }
            break;
        case 0xED:
            switch (x) {
                case 1:
                    switch (z) {
                        case 0:
                            sprintf(output, (y == 6) ? "IN (C)" : "IN %s,(C)", r[y]);
                            break;
                        case 1:
                            sprintf(output, (y == 6) ? "OUT (C)" : "OUT %s,(C)", r[y]);
                            break;
                        case 2:
                            sprintf(output, (q == 0) ? "SBC HL,%s" : "ADC HL,%s", rp[p]);
                            break;
                        case 3:
                            addr = input() | (input() << 8);
                            if (q == 0) 
                                sprintf(output, "LD (%04XH),%s", addr, rp[p]);
                            else
                                sprintf(output, "LD %s,(%04XH)", addr, rp[p]);
                            break;
                        case 4:
                            sprintf(output, "NEG");
                            break;
                        case 5:
                            sprintf(output, (y == 1) ? "RETI" : "RETN");
                            break;
                        case 6:
                            sprintf(output, "IM %s", im[y&0x3]);
                            break;
                        case 7:
                            sprintf(output, edx1z7ops[y]);
                            break;
                    }
                    break;
                case 2:
                    if (z <= 3 && y >= 4)
                        sprintf(output, bli[yz]);
                    else
                        sprintf(output, "illegal opcode");
                    break;
                default:
                    sprintf(output, "illegal opcode");
                    break;
            }
            break;
        default:
            // un-prefixed opcodes
            switch (x) {
                case 0:
                    switch (z) {
                        case 0:
                            switch (y) {
                                case 0:
                                    sprintf(output, "NOP");
                                    break;
                                case 1:
                                    sprintf(output, "EX AF,AF'");
                                    break;
                                case 2:
                                    addr += (int8_t)input() + 2;
                                    sprintf(output, "DJNZ %04XH", addr);
                                    break;
                                case 3:
                                    addr += (int8_t)input() + 2;
                                    sprintf(output, "JR %04XH", addr);
                                    break;
                                default:
                                    addr += (int8_t)input() + 2;
                                    sprintf(output, "JR %s,%04XH", cc[y-4], addr);
                                    break;
                            }
                            break;
                        case 1:
                            if (q == 0) {
                                addr = input() | (input() << 8);
                                sprintf(output, "LD %s,%04XH", rp[p], addr);
                            } else
                                sprintf(output, "ADD HL,%s", rp[p]);
                            break;
                        case 2:
                            if (y < 4)
                                sprintf(output, ldops[y]);
                            else {
                                addr = input() | (input() << 8);
                                sprintf(output, ldops[y], addr); 
                            }
                            break;
                        case 3:
                            sprintf(output, (q == 0) ? "INC %s" : "DEC %s", rp[p]);
                            break;
                        case 4:
                            sprintf(output, "INC %s", r[y]);
                            break;
                        case 5:
                            sprintf(output, "DEC %s", r[y]);
                            break;
                        case 6:
                            imm = input();
                            sprintf(output, "LD %s,%02XH", r[y], imm);
                            break;
                        case 7:
                            sprintf(output, afops[y]);
                            break;
                    }
                    break;
                case 1:
                    if (z == 6 && y == 6)
                        sprintf(output, "HALT");
                    else
                        sprintf(output, "LD %s,%s", r[y], r[z]);
                    break;
                case 2:
                    sprintf(output, alu[y], r[z]);
                    break;
                case 3:
                    switch (z) {
                        case 0:
                            sprintf(output, "RET %s", cc[y]);
                            break;
                        case 1: 
                            if (q == 0)
                                sprintf(output, "POP %s", p < 3 ? rp[p] : rp[4]);
                            else
                                sprintf(output, x3z1q1ops[p]);
                            break;
                        case 2:
                            addr = input() | (input() << 8);
                            sprintf(output, "JP %s,%04XH", cc[y], addr);
                            break;
                        case 3:
                            switch(y) {
                                case 0:
                                    addr = input() | (input() << 8);                                    
                                    sprintf(output, "JP %04XH", addr);
                                    break;
                                case 1:
                                    break;
                                case 2:
                                    imm = input();
                                    sprintf(output, "OUT (%02XH),A", imm);
                                    break;
                                case 3:
                                    imm = input();
                                    sprintf(output, "IN A,(%02XH)", imm);
                                    break;
                                default:
                                    sprintf(output, exintops[y-4]);
                                    break;
                            }
                            break;
                        case 4:
                            addr = input() | (input() << 8);
                            sprintf(output, "CALL %s,%04XH", cc[y], addr);
                            break;
                        case 5:
                            if (q == 0)
                                sprintf(output, "PUSH %s", p < 3 ? rp[p] : rp[4]);
                            else if (p == 0) {
                                addr = input() | (input() << 8);
                                sprintf(output, "CALL %04XH", addr);
                            } 
                            break;
                        case 6:
                            imm = input();
                            sprintf(output, "%s %02XH", alu[y], imm);
                            break;
                        case 7:
                            sprintf(output, "RST %02XH", y*8);
                            break;
                    }
                    break;
            }
            break;
    }
}