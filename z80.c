#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "diskemu.h"

void z80_reset(void)
{
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    IOACK_LO;
    IOACK_HI;
}

void z80_run(uint16_t addr) {
    uint8_t reset_vect[] = { 0xc3, (addr & 0xFF), (addr >> 8) };
    if (addr > 0x0002) {
        write_mem(0x0000, reset_vect, 3);
    }
    z80_reset();
    clk_run();
    for (;;) {
        if (!GET_IORQ) {
            clk_stop();
            switch (GET_ADDRLO) {
                case 0x10:             // sio0 status
                    if (!GET_RD) {
                        SET_DATA(((UCSR0A >> (UDRE0 - 1)) & 0x2) | ((UCSR0A >> RXC0) & 0x1));
                        DATA_OUTPUT;
                    }
                    break;
                case 0x11:             // sio0 data
                    if (!GET_RD) {
                        SET_DATA(UDR0);
                        DATA_OUTPUT;
                    } else if (!GET_WR) {
                        UDR0 = GET_DATA;
                    }
                    break;
                case DRIVE_STATUS:
                    if (!GET_RD) {
                        SET_DATA(drive_status());
                        DATA_OUTPUT;
                    } else if (!GET_WR) {
                        drive_select(GET_DATA);
                    }
                    break;
                case DRIVE_CONTROL:
                    if (!GET_RD) {
                        SET_DATA(drive_sector());
                        DATA_OUTPUT;
                    } else if (!GET_WR) {
                        drive_control(GET_DATA);
                    }
                    break;
                case DRIVE_DATA:
                    if (!GET_RD) {
                        SET_DATA(drive_read());
                        DATA_OUTPUT;
                    } else if (!GET_WR) {
                        drive_write(GET_DATA);
                    }
                    break;
                default:
                    if (!GET_RD) {
                        SET_DATA(0xFF);
                    }
            }
            IOACK_LO;
            while (!GET_IORQ)
                CLK_TOGGLE;
            DATA_INPUT;
            IOACK_HI;
            clk_run();
        }
        if (!GET_HALT) {
            clk_stop();
            CLK_LO;
            printf_P(PSTR("\n\nz80 halted\n"));
            break;
        }
    }
}

// Trace the bus state for specified number of clock cycles
void z80_trace(int cycles)
{
}