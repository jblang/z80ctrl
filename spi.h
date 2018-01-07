#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init();
inline uint8_t spi_exchange(uint8_t val);

#endif
