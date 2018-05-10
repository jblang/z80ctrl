#include "defines.h"

#include "bus.h"
#include "iox.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdio.h>

void clk_cycle(uint8_t cycles)
{
    uint8_t i;
    for (i = 0; i < cycles; i++) {
        CLK_HI;
        CLK_LO;
    }
}

void clk_trace(uint8_t cycles)
{
    uint8_t i;
    for (i = 0; i < cycles; i++) {
        CLK_HI;
        CLK_LO;
        bus_status();
    }
}

// Run the Z80's clock
void clk_run(void)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = 1;
    OCR2B = 0;
}

// Stop the Z80's clock
void clk_stop()
{
    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = 0;
    OCR2B = 0;
}

// Request control of the bus from the Z80
uint8_t bus_master(void)
{
    uint8_t i = 255;

    BUSRQ_LO;           // request bus
    IOACK_LO;           // make sure not in WAIT state
    IOACK_HI;
    while (GET_BUSACK && i--)  // wait for BUSACK to go low
        CLK_TOGGLE;
    if (i == 0) {
        printf("bus request timed out\n");
        BUSRQ_HI;
        return 0;
    }
    MREQ_HI;
    RD_HI;
    WR_HI;
    MREQ_OUTPUT;
    RD_OUTPUT;
    WR_OUTPUT;
    ADDR_OUTPUT;
    DATA_INPUT;
    return 1;
}

// Return control of the bus to the Z80
void bus_slave(void)
{
    MREQ_INPUT;
    RD_INPUT;
    WR_INPUT;
    ADDR_INPUT;
    DATA_INPUT;
    BUSRQ_HI;
}

// Log current bus status
bus_stat bus_status()
{
    bus_stat status;
    status.flags.bytes.lo = GET_XFLAGS;
    status.flags.bytes.hi = GET_BFLAGS | GET_DFLAGS;
    status.data = GET_DATA;
    status.addr = GET_ADDR;
    return status;
}

// Initialize bus
void bus_init(void)
{
    // Initialize I/O expander
    iox_init();

    // Configure bus signal direction
    INT_OUTPUT;
    NMI_OUTPUT;
    RESET_OUTPUT;
    BUSRQ_OUTPUT;
#ifdef BANK_OUTPUT
    BANK_OUTPUT;
#endif
    IOACK_OUTPUT;
    CLK_OUTPUT;

    IORQ_INPUT;
    BUSACK_INPUT;
    M1_INPUT;
#ifdef RFSH_INPUT
    RFSH_INPUT;
#endif
    HALT_INPUT;

    // Pullup on halt so it can be used with a switch
    HALT_PULLUP;

    // Set defaults for output bus signals
    INT_HI;
    NMI_HI;
    BUSRQ_HI;
#ifdef SET_BANK
    SET_BANK(0);
#endif

    // Reset the processor
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    IOACK_LO;
    IOACK_HI;

    // Make bidirectional signals inputs
    bus_slave();
}
