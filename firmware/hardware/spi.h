/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file spi.h SPI chip select pin definitions
 */

#ifndef HARDWARE_SPI_H
#define HARDWARE_SPI_H

#include <stdint.h>
#include <avr/io.h>

// SPI pins
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_PIN PINB

#define SCK DDB7
#define MISO DDB6
#define MOSI DDB5

#define IOX_ADDR 0
#define SD_ADDR 1
#define AUX1_ADDR 2
#define AUX2_ADDR 3

#define CSADDR 3
#define CSADDRMASK ((1 << CSADDR) | (1 << CSADDR+1))

#define IOX_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (IOX_ADDR << CSADDR)
#define SD_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (SD_ADDR << CSADDR)
#define AUX1_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (AUX1_ADDR << CSADDR)
#define AUX2_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (AUX2_ADDR << CSADDR)

#define SPI_SLOW SPCR |= ((1 << SPR1) | (1 << SPR0));
#define SPI_FAST SPCR &= ~((1 << SPR1) | (1 << SPR0))

#define SPI_PHASE0 SPCR &= ~(1 << CPHA)
#define SPI_PHASE1 SPCR |= (1 << CPHA)

#define SPI_ENABLE SPCR |= (1 << SPE)
#define SPI_DISABLE SPCR &= ~(1 << SPE)

#define MISO_LO SPI_PORT &= ~(1 << MISO)
#define MISO_HI SPI_PORT |= (1 << MISO)
#define GET_MISO (SPI_PIN & (1 << MISO))
#ifdef MISO_INPUT_PULLUP
#define MISO_INPUT SPI_DDR &= ~(1 << MISO); MISO_HI;
#define MISO_OUTPUT SPI_DDR |= (1 << MISO); MISO_LO;
#else
#define MISO_INPUT SPI_DDR &= ~(1 << MISO)
#define MISO_OUTPUT SPI_DDR |= (1 << MISO)
#endif


#define SCK_LO SPI_PORT &= ~(1 << SCK)
#define SCK_HI SPI_PORT |= (1 << SCK)

void spi_init();
uint8_t spi_exchange(uint8_t val);

#endif
