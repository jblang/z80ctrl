#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>

uint8_t disasm(uint16_t addr, uint8_t (*input)(), char *output);
void disasm_mem(uint16_t start, uint16_t end);

#endif