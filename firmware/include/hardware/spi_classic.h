#ifndef SPI_1284P_H
#define SPI_1284P_H
// z80ctrl classic using ATmega1284p

#include <avr/io.h>

// SPI control registers
#define SPI_SLOW SPCR |= ((1 << SPR1) | (1 << SPR0))
#define SPI_FAST SPCR &= ~((1 << SPR1) | (1 << SPR0))

#define SPI_PHASE0 SPCR &= ~(1 << CPHA)
#define SPI_PHASE1 SPCR |= (1 << CPHA)

#define SPI_ENABLE SPCR |= (1 << SPE)
#define SPI_DISABLE SPCR &= ~(1 << SPE)

#define SPI_MASTER SPCR = (1 << MSTR)

#define SPI_DATA SPDR
#define SPI_READY (SPSR & (1 << SPIF))

// SPI bus pins
#define SPI_PORT B

#define SPI_SCK_PIN DDB7
#define SPI_MISO_PIN DDB6
#define SPI_MOSI_PIN DDB5

#define MISO_LO PORT ## SPI_PORT &= ~(1 << MISO)
#define MISO_HI PORT ## SPI_PORT |= (1 << MISO)
#define GET_MISO (PIN ## SPI_PORT & (1 << MISO))
#ifdef MISO_INPUT_PULLUP
#define MISO_INPUT           \
    DDR ## SPI_PORT &= ~(1 << MISO); \
    MISO_HI;
#define MISO_OUTPUT         \
    DDR ## SPI_PORT |= (1 << MISO); \
    MISO_LO;
#else
#define MISO_INPUT DDR ## SPI_PORT &= ~(1 << MISO)
#define MISO_OUTPUT DDR ## SPI_PORT |= (1 << MISO)
#endif

#define SCK_LO PORT ## SPI_PORT &= ~(1 << SCK)
#define SCK_HI PORT ## SPI_PORT |= (1 << SCK)

#define SPI_DIR_CFG \
    DDR ## SPI_PORT &= ~(1 << MISO); \
    DDR ## SPI_PORT |= (1 << MOSI) | (1 << SCK); \
    DDR ## CS_PORT |= CSADDR_MASK;
#endif