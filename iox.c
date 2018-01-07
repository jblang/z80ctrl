#include "defines.h"

#include "iox.h"
#include "spi.h"

void iox_begin(uint8_t mode, uint8_t addr)
{
    SPI_PORT &= ~(1 << IOX_CS);
    spi_exchange(SPI_ADDR | mode);
    spi_exchange(addr);
}

void iox_end(void)
{
    SPI_PORT |= (1 << IOX_CS);
}

uint8_t iox_read(uint8_t addr)
{
    uint8_t data;
    iox_begin(READ, addr);
    data = spi_exchange(0);
    iox_end();
    return data;
}

void iox_write(uint8_t addr, uint8_t data)
{
    iox_begin(WRITE, addr);
    spi_exchange(data);
    iox_end();
}
