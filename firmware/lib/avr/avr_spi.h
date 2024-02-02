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

#ifndef AVR_SPI_H
#define AVR_SPI_H

#include <stdint.h>

// Chip select addresses
typedef enum {
    CS_IOX = 0,
    CS_MMC = 1,
    CS_RTC = 2,
    CS_IDLE = 3
} spi_cs_t;

typedef enum {
    SPI_SLOW,
    SPI_FAST
} spi_speed_t;

typedef enum {
    SPI_PHASE0,
    SPI_PHASE1
} spi_phase_t;

void spi_init();
void spi_cs(spi_cs_t chip);
void spi_speed(spi_speed_t speed);
void spi_phase(spi_phase_t phase);

uint8_t spi_exchange(uint8_t value);
void spi_send_buf(const uint8_t *buf, uint16_t len);
void spi_receive_buf(uint8_t *buf, uint16_t len);

void miso_multiplex_begin();
void miso_multiplex_end();
uint8_t miso_multiplex_in();
void miso_multiplex_out(uint8_t value);

#endif
