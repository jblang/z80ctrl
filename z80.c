#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "z80.h"
#include "bus.h"
#include "diskemu.h"
#include "opcodes.h"

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

#define HL(signal) ((signal) ? 'H' : 'L')

// Log current 
void z80_status()
{
    uint8_t data = GET_DATA;
    uint16_t addr = GET_ADDR;

    // Log current bus operation
    if (!GET_M1)
        printf("op fetch\t");
    else if (!GET_MREQ && !GET_RD)
        printf("mem read\t");
    else if (!GET_MREQ && !GET_WR)
        printf("mem write\t");
    else if (!GET_IORQ && !GET_RD)
        printf("io read\t\t");
    else if (!GET_IORQ && !GET_WR)
        printf("io write\t");

    // Log current address and data byte
    printf("%04x\t%02x ", GET_ADDR, GET_DATA);

    // Handle instruction fetch
    if (!GET_M1)
        // Get remaining instruction bytes
        // Lookup instruction length and mnemonic for opcode
        if (opcodes[data][0] == '2') {
            // Two-byte operand (address)
            clk_cycle(3);
            addr = GET_DATA;
            printf("%02x ", GET_DATA);
            clk_cycle(3);
            addr |= GET_DATA << 8;
            printf("%02x\t", GET_DATA);
            printf(opcodes[data]+1, addr);
        } else if (opcodes[data][0] == '1') {
            // One-byte operand (immediate)
            clk_cycle(3);
            addr = GET_DATA;
            printf("%02x\t\t", GET_DATA);
            printf(opcodes[data]+1, addr);
        } else if (opcodes[data][0] == '+') {
            // One-byte operand (relative address)
            clk_cycle(3);
            addr += (int8_t)GET_DATA + 2;
            printf("%02x\t\t", GET_DATA);
            printf(opcodes[data]+1, addr);
        } else if (opcodes[data][0] == '*') {
            // Two-byte opcode
            printf("\t\t%s", opcodes[data]);
        } else {
            printf("\t\t%s", opcodes[data]);
        }

    // If not instruction fetch, output printable ASCII chacters
    else if (0x20 <= data && data <= 0x7e)
        printf("\t\t%c", data);

    printf("\n");

    // Finish current memory cycle
    while (!GET_MREQ) {
        CLK_LO;
        CLK_HI;
    }

    // wait until output is fully transmitted to avoid
    // interfering with UART status for running program
    loop_until_bit_is_set(UCSR0A, UDRE0);
}

#define MEM_DEBUG (opfetch_watch_start <= opfetch_watch_end || opfetch_break_start <= opfetch_break_end || memrd_watch_start <= memrd_watch_end || memrd_break_start <= memrd_break_end || memwr_watch_start <= memwr_watch_end || memwr_break_start <= memwr_break_end)

// Trace reads and writes for the specified number of instructions
void z80_trace(uint32_t cycles)
{
    uint32_t c = 0;
    uint8_t brk = 0;
    while (GET_HALT && (cycles == 0 || c < cycles) && !brk) {
        CLK_LO;
        CLK_HI;
        if (!GET_IORQ) {
            uint8_t addr = GET_ADDRLO;
            if ((!GET_WR && iowr_watch_start <= addr && addr <= iowr_watch_end) ||
                (!GET_RD && iord_watch_start <= addr && addr <= iord_watch_end))
                z80_status();
            if ((!GET_WR && iowr_break_start <= addr && addr <= iowr_break_end) ||
                (!GET_RD && iord_break_start <= addr && addr <= iord_break_end))
                brk = 1;
            z80_iorq();
        } else if (!GET_MREQ && MEM_DEBUG) {
            uint16_t addr = GET_ADDR;
            if ((!GET_M1 && opfetch_watch_start <= addr && addr <= opfetch_watch_end) ||
                (!GET_RD && memrd_watch_start <= addr && addr <= memrd_watch_end) ||
                (!GET_WR && memwr_watch_start <= addr && addr <= memwr_watch_end))
                z80_status();
            if ((!GET_M1 && opfetch_break_start <= addr && addr <= opfetch_break_end) ||
                (!GET_RD && memrd_break_start <= addr && addr <= memrd_break_end) ||
                (!GET_WR && memwr_break_start <= addr && addr <= memwr_break_end))
                brk = 1;
        }
        c++;
    }
}