#ifndef Z80_H
#define Z80_H

#include <stdint.h>

void z80_reset(void);
void z80_run(uint16_t addr);
void z80_trace(int cycles);

#endif