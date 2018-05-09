#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>

uint8_t disasm(uint16_t addr, uint8_t (*input)(), char *output);

#endif