#include "disasm.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// reference: http://www.z80.info/decoding.htm

// first byte of mnemonic encodes instruction length:
// 1 = single byte immediate operand
// 2 = two byte address operand
// + = one byte relative address operand
// * = opcode prefix
// other = single byte instruction (no operand)

// 8-bit registers
char *r[] = {
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "(HL)",
    "A"
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
    "0/1",
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

// misc ops for x = 0, z = 0
char *x0z0ops[] = {
    "NOP",
    "EX AF,AF'",
    "+DJNZ %04XH",
    "+JR %04XH",
    "+JR %s,%s"
};

// load ops for x = 0, z = 2
char *x0z2ops[] = {
    "LD (BC),A",
    "LD A,(BC)",
    "LD (DE),A",
    "LD A,(DE)",
    "2LD (%04XH),HL",
    "2LD HL,(%04XH)",
    "2LD (%04XH),A",
    "2LD A,(%04XH)"        
};

// accumulator and flag ops for x = 0, z = 7
char *x0z7ops[] = {
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
char *x3z3ops[] = {
    "2JP %04XH",
    "*CB",
    "1OUT (&%02X),A",
    "1IN A,(&%02X)",
    "EX (SP),HL",
    "EX DE,HL",
    "DI",
    "EI"
};

uint8_t disasm(uint8_t prefix, uint8_t opcode, char *mnemonic)
{
    // bit slice the opcode
    uint8_t x = (opcode & 0xc0) >> 6;       // x = opcode[7:6]
    uint8_t y = (opcode & 0x3a) >> 3;       // y = opcode[5:3]
    uint8_t z = (opcode & 0x07);            // z = opcode[2:0]
    uint8_t p = (opcode & 0x30) >> 4;       // p = opcode[5:4]
    uint8_t q = (opcode & 0x08) >> 3;       // q = opcode[3]
    uint8_t yz = ((y & 3) << 2) | (z & 3);  // yz = {opcode[1:0], opcode[4:3]}

    //printf(" [%X %X %X <%X:%X> %X] ", x, z, y, p, q, yz);
    switch (prefix) {
        default:
            // un-prefixed opcodes
            switch (x) {
                case 0:
                    switch (z) {
                        case 0:
                            if (y < 4)
                                // assorted ops
                                strcpy(mnemonic, x0z0ops[y]);
                            else
                                // relative jump
                                sprintf(mnemonic, x0z0ops[4], cc[y-4], "%04XH");
                            break;
                        case 1:
                            if (q == 0)
                                // 16-bit immediate load
                                sprintf(mnemonic, "2LD %s,%s", rp[p], "%04XH");
                            else
                                // 16-bid immediate add
                                sprintf(mnemonic, "ADD HL,%s", rp[p]);
                            break;
                        case 2:
                            // indirect loading
                            strcpy(mnemonic, x0z2ops[y]);
                            break;
                        case 3:
                            // 16-bit inc/dec
                            sprintf(mnemonic, (q == 0) ? "INC %s" : "DEC %s", rp[p]);
                            break;
                        case 4:
                            // 8-bit inc
                            sprintf(mnemonic, "INC %s", r[y]);
                            break;
                        case 5:
                            // 8-bit dec
                            sprintf(mnemonic, "DEC %s", r[y]);
                            break;
                        case 6:
                            // 8-bit load immediate
                            sprintf(mnemonic, "1LD %s,%s", r[y], "&%02X");
                            break;
                        case 7:
                            // assorted operations on accumulator/flags
                            strcpy(mnemonic, x0z7ops[y]);
                            break;
                    }
                    break;
                case 1:
                    if (z == 6 && y == 6)
                        // exception: halt replaces ld (hl),(hl)
                        strcpy(mnemonic, "HALT");
                    else
                        // 8-bit loading
                        sprintf(mnemonic, "LD %s,%s", r[y], r[z]);
                    break;
                case 2:
                    // operate on accumulator and register/memory location
                    sprintf(mnemonic, alu[y], r[z]);
                    break;
                case 3:
                    switch (z) {
                        case 0:
                            // conditional return
                            sprintf(mnemonic, "RET %s", cc[y]);
                            break;
                        case 1: 
                            if (q == 0)
                                // pop to register pair
                                sprintf(mnemonic, "POP %s", p < 3 ? rp[p] : rp[4]);
                            else
                                // various operations
                                strcpy(mnemonic, x3z1q1ops[p]);
                            break;
                        case 2:
                            // conditional jump
                            sprintf(mnemonic, "2JP %s,%s", cc[y], "%04XH");
                            break;
                        case 3:
                            // assorted operations
                            strcpy(mnemonic, x3z3ops[y]);
                            break;
                        case 4:
                            // conditional call
                            sprintf(mnemonic, "2CALL %s,%s", cc[y], "%04XH");
                            break;
                        case 5:
                            if (q == 0)
                                // push register pair
                                sprintf(mnemonic, "PUSH %s", p < 4 ? rp[p] : rp[5]);
                            else if (p == 0)
                                // unconditional call
                                strcpy(mnemonic, "2CALL %04XH");
                            else
                                // opcode prefixes
                                sprintf(mnemonic, "*%02X", opcode);
                            break;
                        case 6:
                            // operate on accumulator and immediate operand
                            mnemonic[0] = '1';
                            sprintf(mnemonic+1, alu[y], "&%02X");
                            break;
                        case 7:
                            // restart
                            sprintf(mnemonic, "RST &%02X", y*8);
                            break;
                    }
                    break;
            }
            break;
    }
}
