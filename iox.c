#include "defines.h"

#include "iox.h"
#include "spi.h"

void iox_init(void)
{
    spi_init();
    iox_write(GPPUA0, 0xFF);
    iox_write(GPPUB0, 0xFF);
}

void iox_begin(uint8_t mode, uint8_t addr)
{
    IOX_SEL;
    spi_exchange(SPI_ADDR | mode);
    spi_exchange(addr);
}

void iox_end(void)
{
    AUX2_SEL;
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
