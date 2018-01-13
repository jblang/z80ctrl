#include "defines.h"

#include "bus.h"
#include "iox.h"

#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>

void clk_cycle(uint8_t cycles)
{
    CLK_LO;
    uint8_t i;
    for (i = 0; i < cycles; i++) {
        CLK_HI;
        CLK_LO;
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
void bus_master(void)
{
    BUSRQ_LO;           // request bus
    IOACK_LO;           // make sure not in WAIT state
    IOACK_HI;
    while (GET_BUSACK)  // wait for BUSACK to go low
        CLK_TOGGLE;
    CTRL_HI;
    CTRL_OUTPUT;        
    ADDR_OUTPUT;
    DATA_INPUT;
}

// Return control of the bus to the Z80
void bus_slave(void)
{
    CTRL_INPUT;
    ADDR_INPUT;
    DATA_INPUT;
    BUSRQ_HI;
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
    BANK_OUTPUT;
    IOACK_OUTPUT;
    CLK_OUTPUT;

    IORQ_INPUT;
    BUSACK_INPUT;
    M1_INPUT;
    RFSH_INPUT;
    HALT_INPUT;
    
    // Pullup on halt so it can be used with a switch
    HALT_PULLUP;

    // Set defaults for output bus signals
    INT_HI;
    NMI_HI;
    BUSRQ_HI;
    SET_BANK(0);

    // Reset the processor
    z80_reset();

    // Make bidirectional signals inputs
    bus_slave();
}

// Log current bus status
void bus_status(void)
{
    uint8_t data = GET_DATA;
    uint8_t ascii = (data >= 0x20 && data <= 0x7e) ? data : ' ';

    printf(
#ifdef M1
        "m1=%x "
#else
        "m1=X "
#endif        
        "mreq=%x iorq=%x rd=%x wr=%x "
#ifdef RFSH
        "rfsh=%x "
#else
        "rfsh=X "
#endif
#ifdef HALT
        "halt=%x "
#else
        "halt=X "
#endif
        "wait=%x "
        "int=%x nmi=%x reset=%x busrq=%x busack=%x "
        "clk=%x bank=%x addr=%04x data=%02x %c\n",
#ifdef M1
        !!GET_M1, 
#endif
        !!GET_MREQ, !!GET_IORQ, !!GET_RD, !!GET_WR,
#ifdef RFSH
        !!GET_RFSH, 
#endif
#ifdef HALT
        !!GET_HALT, 
#endif
        !!GET_IOACK,
        !!GET_INT, !!GET_NMI, !!GET_RESET, !!GET_BUSRQ, !!GET_BUSACK,
        !!GET_CLK, GET_BANK, GET_ADDR, data, ascii);
}