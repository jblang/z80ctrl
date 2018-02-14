#include "defines.h"

#include "bus.h"
#include "iox.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
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
    MREQ_HI;
    RD_HI;
    WR_HI;
    MREQ_OUTPUT;
    RD_OUTPUT;
    WR_OUTPUT;
    ADDR_OUTPUT;
    DATA_INPUT;
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

#define HL(signal) ((signal) ? 'H' : 'L')

// Log current bus status
void bus_status()
{
    uint8_t ctrlx = iox_read(CTRLX_GPIO);
    uint8_t data = GET_DATA;

    printf_P(
        PSTR("clk=%c m1=%c mreq=%c iorq=%c ioack=%c rd=%c wr=%c rfsh=%c halt=%c "
        "int=%c nmi=%c reset=%c busrq=%c busack=%c "
#ifdef BANKMASK
        "bank=%X "
#endif
        "addr=%04X "
        "data=%02X %c\n"),
        HL(GET_CLK),
        HL(GET_M1),
        HL(ctrlx & (1 << MREQ)),
        HL(GET_IORQ),
        HL(GET_IOACK),
        HL(GET_RD),
        HL(GET_WR),
#ifdef RFSH
        HL(ctrlx & (1 << RFSH)),
#else
        'X',
#endif
        HL(GET_HALT), 
        HL(ctrlx & (1 << INTERRUPT)),
        HL(ctrlx & (1 << NMI)),
        HL(ctrlx & (1 << RESET)),
        HL(ctrlx & (1 << BUSRQ)),
        HL(ctrlx & (1 << BUSACK)),
#ifdef BANKMASK
        ((ctrlx & BANKMASK) >> BANKADDR),
#endif
        GET_ADDR,
        data,
        0x20 <= data && data <= 0x7e ? data : ' ');

        // wait until output is fully transmitted to avoid
        // interfering with UART status for running program
        loop_until_bit_is_set(UCSR0A, UDRE0);
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
