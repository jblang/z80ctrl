#ifndef Z80_H
#define Z80_H

#include <stdint.h>

extern uint16_t bus_watch_start;
extern uint16_t bus_watch_end;
extern uint16_t memrd_watch_start;
extern uint16_t memrd_watch_end;
extern uint16_t memwr_watch_start;
extern uint16_t memwr_watch_end;
extern uint8_t iord_watch_start;
extern uint8_t iord_watch_end;
extern uint8_t iowr_watch_start;
extern uint8_t iowr_watch_end;
extern uint16_t opfetch_watch_start;
extern uint16_t opfetch_watch_end;
extern uint16_t memrd_break_start;
extern uint16_t memrd_break_end;
extern uint16_t memwr_break_start;
extern uint16_t memwr_break_end;
extern uint8_t iord_break_start;
extern uint8_t iord_break_end;
extern uint8_t iowr_break_start;
extern uint8_t iowr_break_end;
extern uint16_t opfetch_break_start;
extern uint16_t opfetch_break_end;

void z80_reset(uint16_t addr);
void z80_status(void);
void z80_run(void);
void z80_debug(uint32_t cycles);

#endif