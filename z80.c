#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "diskemu.h"

// Breakpoints and watches
uint16_t memrd_watch_start = 0xffff;
uint16_t memrd_watch_end = 0;
uint16_t memwr_watch_start = 0xffff;
uint16_t memwr_watch_end = 0;
uint8_t iord_watch_start = 0xff;
uint8_t iord_watch_end = 0;
uint8_t iowr_watch_start = 0xff;
uint8_t iowr_watch_end = 0;
uint16_t opfetch_watch_start = 0xffff;
uint16_t opfetch_watch_end = 0;
uint16_t memrd_break_start = 0xffff;
uint16_t memrd_break_end = 0;
uint16_t memwr_break_start = 0xffff;
uint16_t memwr_break_end = 0;
uint8_t iord_break_start = 0xff;
uint8_t iord_break_end = 0;
uint8_t iowr_break_start = 0xff;
uint8_t iowr_break_end = 0;
uint16_t opfetch_break_start = 0xffff;
uint16_t opfetch_break_end = 0;

void z80_reset(uint16_t addr)
{
    uint8_t reset_vect[] = { 0xc3, (addr & 0xFF), (addr >> 8) };
    if (addr > 0x0002) {
        write_mem(0x0000, reset_vect, 3);
    }
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    IOACK_LO;
    IOACK_HI;
}

#define SIO0_STATUS 0x10
#define SIO0_DATA 0x11

void z80_iorq(void)
{
    switch (GET_ADDRLO) {
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA(((UCSR0A >> (UDRE0 - 1)) & 0x2) | ((UCSR0A >> RXC0) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIO0_DATA:
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
    while (!GET_IORQ) {
        CLK_TOGGLE;
    }
    DATA_INPUT;
    IOACK_HI;
}

void z80_run(void)
{
    clk_run();
    while (GET_HALT) {
        if (!GET_IORQ) {
            clk_stop();
            z80_iorq();
            clk_run();
        }
    }
    clk_stop();
    CLK_LO;
}

#define HL(signal) ((signal) ? 'H' : 'L')

// Log current bus status
void z80_status()
{
    uint8_t ctrlx = iox_read(CTRLX_GPIO);
    uint8_t data = GET_DATA;

    printf_P(
        PSTR("clk=%c m1=%c mreq=%c iorq=%c ioack=%c rd=%c wr=%c rfsh=%c halt=%c "
        "int=%c nmi=%c reset=%c busrq=%c busack=%c bank=%X addr=%04X "
        "data=%02X %c\n"),
        HL(GET_CLK), 
        HL(GET_M1), 
        HL(GET_MREQ), 
        HL(GET_IORQ), 
        HL(GET_IOACK),
        HL(GET_RD), 
        HL(GET_WR), 
#ifdef RFSH
        HL(GET_RFSH), 
#else
        'X',
#endif
        HL(GET_HALT), 
        HL(ctrlx & (1 << INTERRUPT)), 
        HL(ctrlx & (1 << NMI)), 
        HL(ctrlx & (1 << RESET)), 
        HL(ctrlx & (1 << BUSRQ)), 
        HL(ctrlx & (1 << BUSACK)),
        ((ctrlx & BANKMASK) >> BANKADDR), 
        GET_ADDR, 
        data,
        0x20 <= data && data <= 0x7e ? data : ' ');

        // wait until output is fully transmitted to avoid
        // interfering with UART status for running program
        loop_until_bit_is_set(UCSR0A, UDRE0);
}

// Trace the bus state for specified number of clock cycles
void z80_trace(uint32_t cycles)
{
    uint32_t c = 0;
    while (GET_HALT && (cycles == 0 || c < cycles)) {
        CLK_HI;
        CLK_LO;
        uint16_t addr = GET_ADDR;
        if (!GET_IORQ) {
            addr &= 0xff;
            if ((!GET_WR && iowr_watch_start <= addr && addr <= iowr_watch_end) ||
                (!GET_RD && iord_watch_start <= addr && addr <= iord_watch_end))
                z80_status();

            z80_iorq();

            if ((!GET_WR && iowr_break_start <= addr && addr <= iowr_break_end) ||
                (!GET_RD && iord_break_start <= addr && addr <= iord_break_end))
                break;
        } else if (!GET_MREQ) {
            if (!GET_M1 && opfetch_watch_start <= addr && addr <= opfetch_watch_end)
                z80_status();
            else if ((!GET_RD && memrd_watch_start <= addr && addr <= memrd_watch_end) ||
                    (!GET_WR && memwr_watch_start <= addr && addr <= memwr_watch_end))
                z80_status();
            if (!GET_M1 && opfetch_break_start <= addr && addr <= opfetch_break_end)
                break;
            else if ((!GET_RD && memrd_break_start <= addr && addr <= memrd_break_end) ||
                    (!GET_WR && memwr_break_start <= addr && addr <= memwr_break_end))
                break;
        }
        c++;
    }
}