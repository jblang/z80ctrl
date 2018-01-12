#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init();
void spi_slow();
void spi_fast();
inline uint8_t spi_exchange(uint8_t val);

#endif
