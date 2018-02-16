#include "defines.h"

#include "spi.h"

#include <avr/io.h>

void spi_init(void)
{
    SPI_DDR |= (1 << MOSI) | (1 << SCK) | CSADDRMASK;
    SPI_DDR &= ~(1 << MISO);
    AUX2_SEL;
    SPSR = (1 << SPI2X);
    SPCR = (1 << SPE) | (1 << MSTR);
    spi_fast();
}

void spi_slow(void)
{
    SPCR |= ((1 << SPR1) | (1 << SPR0));
}

void spi_fast(void)
{
    SPCR &= ~((1 << SPR1) | (1 << SPR0));
}

uint8_t spi_exchange(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}