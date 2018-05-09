#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "diskemu.h"
#include "disasm.h"

// Breakpoints and watches
uint16_t bus_watch_start = 0xffff;
uint16_t bus_watch_end = 0;
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

// Reset the Z80
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
#define SIOA_CONTROL 0x80
#define SIOA_DATA 0x81
#define SIOB_CONTROL 0x82
#define SIOB_DATA 0x83

// Handle Z80 IO request
void z80_iorq(void)
{
    switch (GET_ADDRLO) {
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA(((UCSR0A >> (UDRE0 - 1)) & 0x2) | ((UCSR0A >> RXC0) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIOA_CONTROL:
            if (!GET_RD) {
                // CTS and DCD always high
                SET_DATA((1 << 3) | (1  << 5) | ((UCSR0A >> (UDRE0 - 2)) & 0x4) | ((UCSR0A >> RXC0) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIOB_CONTROL:
            if (!GET_RD) {
                // CTS and DCD always high
                SET_DATA((1 << 3) | (1  << 5) | ((UCSR1A >> (UDRE1 - 2)) & 0x4) | ((UCSR1A >> RXC1) & 0x1));
                DATA_OUTPUT;
            }
            break;
        case SIO0_DATA:
        case SIOA_DATA:
            if (!GET_RD) {
                SET_DATA(UDR0);
                DATA_OUTPUT;
            } else if (!GET_WR) {
                UDR0 = GET_DATA;
            }
            break;
        case SIOB_DATA:
            if (!GET_RD) {
                SET_DATA(UDR1);
                DATA_OUTPUT;
            } else if (!GET_WR) {
                UDR1 = GET_DATA;
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

// Run the Z80 at full speed
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

void z80_buslog(bus_stat status)
{
    printf_P(
        PSTR("\t%04x %02x %c    %s %s    %s %s %s %s %s %s %s %s\n"),
        status.addr,
        status.data,
        0x20 <= status.data && status.data <= 0x7e ? status.data : ' ',
        !status.flags.bits.rd ? "rd  " :
        !status.flags.bits.wr ? "wr  " :
        !status.flags.bits.rfsh ? "rfsh" : "    ",
        !status.flags.bits.mreq ? "mem" :
        !status.flags.bits.iorq ? "io " : "   ",
        !status.flags.bits.m1 ? "m1" : "  ",
        !status.flags.bits.halt ? "halt" : "    ", 
        !status.flags.bits.interrupt ? "int" : "   ",
        !status.flags.bits.nmi ? "nmi" : "   ",
        !status.flags.bits.reset ? "rst" : "   ",
        !status.flags.bits.busrq ? "busrq" : "     ",
        !status.flags.bits.busack ? "busack" : "      ",
        !status.flags.bits.ioack ? "ioack" : "     ");

        // wait until output is fully transmitted to avoid
        // interfering with UART status for running program
        loop_until_bit_is_set(UCSR0A, UDRE0);    
}

#define INRANGE(start, end, test) ((start) <= (test) && (test) <= (end))

uint8_t z80_tick()
{
    uint8_t lastrd = GET_RD;
    uint8_t lastwr = GET_WR;
    uint8_t logged = 0;
    uint8_t brk = 0;

    CLK_LO;
    CLK_HI;
    bus_stat status = bus_status();
    if (!status.flags.bits.mreq) {
        if (lastrd && !status.flags.bits.rd) {
            if (logged = INRANGE(memrd_watch_start, memrd_watch_end, status.addr))
                z80_buslog(status);
            brk = INRANGE(memrd_break_start, memrd_break_end, status.addr);
        } else if (lastwr && !status.flags.bits.wr) {
            if (logged = INRANGE(memwr_watch_start, memwr_watch_end, status.addr))
                z80_buslog(status);
            brk = INRANGE(memwr_break_start, memwr_break_end, status.addr);
        }
    } else if (!status.flags.bits.iorq) {
        if (lastrd && !status.flags.bits.rd) {
            if (logged = INRANGE(iord_watch_start, iord_watch_end, status.addr & 0xff))
                z80_buslog(status);
            brk = INRANGE(iord_break_start, iord_break_end, status.addr & 0xff);
        } else if (lastwr && !status.flags.bits.wr) {
            if (logged = INRANGE(iowr_watch_start, iowr_watch_end, status.addr & 0xff))
                z80_buslog(status);
            brk = INRANGE(iowr_break_start, iowr_break_end, status.addr & 0xff);
        }
        z80_iorq();
    }        
    if (!logged && INRANGE(bus_watch_start, bus_watch_end, status.addr))
        z80_buslog(status);
    
    return brk;
}

uint8_t z80_read()
{
    uint8_t data;
    while (GET_MREQ || GET_RD)
        z80_tick();
    data = GET_DATA;
    while (!GET_MREQ || !GET_RD)
        z80_tick();
    return data;
}

// Trace reads and writes for the specified number of instructions
void z80_debug(uint32_t cycles)
{
    char mnemonic[255];
    uint32_t c = 0;
    uint8_t brk = 0;

    while (GET_HALT && (cycles == 0 || c < cycles) && !brk) {
        brk = z80_tick();
        if (!GET_M1) {
            uint16_t addr = GET_ADDR;
            brk |= (INRANGE(opfetch_break_start, opfetch_break_end, addr));
            if (!GET_MREQ && !GET_RD) {
                c++;
                disasm(addr, z80_read, mnemonic);
                if (INRANGE(opfetch_watch_start, opfetch_watch_end, addr)) {
                    printf("\t%04x\t%s\n", addr, mnemonic);
                    loop_until_bit_is_set(UCSR0A, UDRE0);    
                }
            }
        }
    }
}