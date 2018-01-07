#include "defines.h"

#include "spi.h"

#include <avr/io.h>

void spi_init(uint8_t cs_mask)
{
    SPI_DDR |= cs_mask | (1 << MOSI) | (1 << SCK);
    SPI_DDR &= ~(1 << MISO);
    SPI_PORT |= cs_mask;
    //SPSR = (1<<SPI2X);
    SPCR = (1 << SPE) | (1 << MSTR);
}

uint8_t spi_exchange(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}
