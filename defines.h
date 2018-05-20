#ifndef DEFINES_H
#define DEFINES_H

#include <avr/io.h>

#include "iox.h"

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#ifndef BAUD
#define BAUD  115200
#endif

// SPI pins
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SCK 7
#define MISO 6
#define MOSI 5
#define CSADDR 3
#define CSADDRMASK ((1 << CSADDR) | (1 << CSADDR+1))

#ifdef CORRECT_SPIADDR
#define IOX_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x0 << CSADDR)
#define SD_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x1 << CSADDR)
#define AUX1_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x2 << CSADDR)
#define AUX2_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x3 << CSADDR)
#else
#define IOX_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x0 << CSADDR)
#define SD_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x2 << CSADDR)
#define AUX1_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x1 << CSADDR)
#define AUX2_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x3 << CSADDR)
#endif

// Enable SD card support in fatfs
#define DRV_MMC 0

// Number of emulated disk drives
#define NUMDRIVES 26

#endif
