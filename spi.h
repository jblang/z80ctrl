/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
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

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <avr/io.h>

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

void spi_init();
void spi_slow();
void spi_fast();
uint8_t spi_exchange(uint8_t val);

#endif
