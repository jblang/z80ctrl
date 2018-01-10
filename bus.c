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

void z80_reset(void)
{
    RESET_LO;
    clk_cycle(3);
    RESET_HI;
    IOACK_LO;
    IOACK_HI;
}

// Set up interrupt on falling edge of IORQ
void enable_iorq_int(void)
{
    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);
    sei();
}

// Interrupt handler for IORQ
ISR(INT0_vect)
{
    //if(get_addrlo() == 0)
        putchar(GET_DATA);
    IOACK_LO;
    _delay_us(100);
    IOACK_HI;
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

// Trace the bus state for specified number of clock cycles
void bus_trace(uint16_t cycles)
{
    CLK_LO;
    uint16_t i;
    for (i = 0; i < cycles && GET_HALT; i++) {
        CLK_HI;
        printf("%05d: ", i);
        bus_status();
        CLK_LO;
        printf("%05d: ", i);
        bus_status();
        if (!GET_IORQ) {
            switch (GET_ADDRLO) {
                case 0x00:
                if (!GET_WR) {
                    putchar(GET_DATA);
                } else if (!GET_RD) {
                    SET_DATA(getchar());
                    DATA_OUTPUT;
                }
                break;
            }
            IOACK_LO;
            while(!GET_IORQ) {
                CLK_HI;
                printf("%05d: ", i);
                bus_status();
                CLK_LO;
                printf("%05d: ", i);
                bus_status();
            }
            DATA_INPUT;
            IOACK_HI;
        }
    }
}

// Read specified number of bytes from external memory to a buffer
void read_mem(uint16_t addr, uint8_t *buf, uint16_t len)
{
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        buf[i] = GET_DATA;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    RD_HI;
    MREQ_HI;
}



// Hex dump memory block to console
void dump_mem(uint16_t addr, uint16_t len)
{
    DATA_INPUT;
    MREQ_LO;
    RD_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        printf("%04x %02x\n", GET_ADDR, GET_DATA);
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    RD_HI;
    MREQ_HI;
}


// Write specified number of bytes to external memory from a buffer
void write_mem(uint16_t addr, uint8_t *buf, uint16_t len)
{
    DATA_OUTPUT;
    MREQ_LO;
    SET_ADDR(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        SET_DATA(buf[i]);
        WR_LO;
        WR_HI;
        addr++;
        if ((addr & 0xFF) == 0) {
            SET_ADDR(addr);
        } else {
            SET_ADDRLO(addr & 0xFF);
        }
    }
    MREQ_HI;
    DATA_INPUT;
}