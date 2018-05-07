#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>

uint8_t disasm(uint8_t prefix, uint8_t opcode, char *mnemonic);

#endif