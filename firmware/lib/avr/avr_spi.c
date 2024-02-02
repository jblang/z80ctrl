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
 * @file spi.c Low-level SPI convenience functions
 */

#include <avr/io.h>
#include <util/delay.h>

#include "avr_spi.h"
#include "avr_gpio.h"

#if __AVR_ARCH__ < 100
// Classic AVR (ATmega1284P)

// SPI control macros
#define SPI_ENABLE SPCR |= (1 << SPE)
#define SPI_DISABLE SPCR &= ~(1 << SPE)
#define SPI_MASTER SPCR = (1 << MSTR)
#define SPI_DATA SPDR
#define SPI_READY (SPSR & (1 << SPIF))

// SPI pin macros
#define SPI_OUTPUT(V) GPIO_OUTPUT(B, (V))
#define SPI_INPUT(V) GPIO_INPUT(B, (V))
#define SPI_PULLUP(V) GPIO_PULLUP_ON(B, (V))
#define SPI_SET(V) GPIO_SET(B, (V))
#define SPI_CLEAR(V) GPIO_CLEAR(B, (V))
#define SPI_READ GPIO_READ(B)

#define SCK_PIN GPIO_PIN7
#define MISO_PIN GPIO_PIN6
#define MOSI_PIN GPIO_PIN5

// Chip select pin macros
#define CS_ADDRPOS 3
#define CS_ADDRMASK (0b11 << CS_ADDRPOS)
#define CS_OUTPUT GPIO_OUTPUT(B, CS_ADDRMASK)
#define CS_READ (GPIO_READ(B))
#define CS_WRITE(V) GPIO_WRITE(B, (V))

#else

// Modern AVR (AVR128DB28/64)

// SPI control macros
#define SPI_INST SPI0
#define SPI_ENABLE SPI_INST.CTRLA |= SPI_ENABLE_bm
#define SPI_DISABLE SPI_INST.CTRLA &= ~SPI_ENABLE_bm
#define SPI_MASTER SPI_INST.CTRLA = SPI_MASTER_bm
#define SPI_DATA SPI_INST.DATA
#define SPI_READY (SPI_INST.INTFLAGS & SPI_RXCIF_bm)

// SPI pin macros
#define SPI_OUTPUT(V) GPIO_OUTPUT(A, (V))
#define SPI_INPUT(V) GPIO_INPUT(A, (V))
#define SPI_PULLUP(V) GPIO_PULLUP_ON(A, (V))
#define SPI_SET(V) GPIO_SET(A, (V))
#define SPI_CLEAR(V) GPIO_CLEAR(A, (V))
#define SPI_READ GPIO_READ(A)

#define MISO_PIN GPIO_PIN5
#define MOSI_PIN GPIO_PIN4
#define SCK_PIN GPIO_PIN6

// Chip select pin macros
#define IOX_CS_OUTPUT GPIO_OUTPUT(D, GPIO_PIN7)
#define IOX_CS_LO GPIO_CLEAR(D, GPIO_PIN7)
#define IOX_CS_HI GPIO_SET(D, GPIO_PIN7)

#define MMC_CS_OUTPUT GPIO_OUTPUT(A, GPIO_PIN7)
#define MMC_CS_LO GPIO_CLEAR(A, GPIO_PIN7)
#define MMC_CS_HI GPIO_SET(A, GPIO_PIN7)

#define RTC_CS_OUTPUT GPIO_OUTPUT(D, GPIO_PIN6)
#define RTC_CS_LO GPIO_CLEAR(D, GPIO_PIN6)
#define RTC_CS_HI GPIO_SET(D, GPIO_PIN6)

#define CS_OUTPUT  \
    IOX_CS_OUTPUT; \
    MMC_CS_OUTPUT; \
    RTC_CS_OUTPUT;

#endif

#define MISO_OUTPUT SPI_OUTPUT(MISO_PIN)
#define MISO_INPUT SPI_INPUT(MISO_PIN)
#define MISO_PULLUP SPI_PULLUP(MISO_PIN)
#define MISO_LO SPI_CLEAR(MISO_PIN)
#define MISO_HI SPI_SET(MISO_PIN)
#define GET_MISO (SPI_READ & MISO_PIN)

#define SCK_OUTPUT SPI_OUTPUT(SCK_PIN)
#define SCK_LO SPI_CLEAR(SCK_PIN)
#define SCK_HI SPI_SET(SCK_PIN)

#define MOSI_OUTPUT SPI_OUTPUT(MOSI_PIN)
#define MOSI_HI SPI_SET(MOSI_PIN)

#define SPI_WAIT while (!SPI_READY)

void spi_cs(spi_cs_t chip)
{
#if BOARD_REV <= 6
    CS_WRITE((CS_READ & ~CS_ADDRMASK) | (chip << CS_ADDRPOS));
#else
    switch (chip)
    {
    case IOX_ADDR:
        IOX_CS_LO;
        break;
    case MMC_ADDR:
        MMC_CS_LO;
        break;
    case RTC_ADDR:
        RTC_CS_LO;
        break;
    case IDLE_ADDR:
    default:
        IOX_CS_HI;
        MMC_CS_HI;
        RTC_CS_HI;
    }
#endif
}

void spi_init(void)
{
    spi_cs(CS_IDLE);
    CS_OUTPUT;
    MISO_PULLUP;
    MISO_INPUT;
    MOSI_OUTPUT;
    SCK_OUTPUT;
    SPI_MASTER;
    spi_speed(SPI_FAST);
    spi_phase(SPI_PHASE0);
    SPI_ENABLE;
}

void spi_speed(spi_speed_t speed)
{
#if __AVR_ARCH__ < 100 // Classic AVR
    if (speed == SPI_SLOW)
        SPCR |= ((1 << SPR1) | (1 << SPR0));
    else
        SPCR &= ~((1 << SPR1) | (1 << SPR0));
#else // Modern AVR
    if (speed == SPI_SLOW)
        SPI_INST.CTRLA = (SPI_INST.CTRLA & ~SPI_PRESC_gm) | SPI_PRESC_DIV128_gc | SPI_CLK2X_bm;
    else
        SPI_INST.CTRLA = (SPI_INST.CTRLA & ~SPI_PRESC_gm) | SPI_PRESC_DIV4_gc | SPI_CLK2X_bm;
#endif
}

void spi_phase(spi_phase_t phase)
{
#if __AVR_ARCH__ < 100 // Classic AVR
    if (phase == SPI_PHASE0)
        SPCR &= ~(1 << CPHA);
    else
        SPCR |= (1 << CPHA);
#else // Modern AVR
    if (phase == SPI_PHASE0)
        SPI_INST.CTRLB = (SPI_INST.CTRLB & ~SPI_MODE_gm) | SPI_MODE_0_gc;
    else
        SPI_INST.CTRLB = (SPI_INST.CTRLB & ~SPI_MODE_gm) | SPI_MODE_1_gc;
#endif
}

uint8_t spi_exchange(uint8_t data)
{
    SPI_DATA = data;
    SPI_WAIT;
    return SPI_DATA;
}

void spi_receive_buf(uint8_t *buf, uint16_t len)
{
    do
    {
        SPI_DATA = 0xff;
        SPI_WAIT;
        *buf++ = SPI_DATA;
        SPI_DATA = 0xff;
        SPI_WAIT;
        *buf++ = SPI_DATA;
    } while (len -= 2);
}

void spi_send_buf(const uint8_t *buf, uint16_t len)
{
    do
    {
        SPI_DATA = *buf++;
        SPI_WAIT;
        SPI_DATA = *buf++;
        SPI_WAIT;
    } while (len -= 2);
}

void miso_multiplex_begin()
{
    MISO_PULLUP;
    MOSI_HI;
    SCK_LO;
    SPI_DISABLE;
    _delay_us(4);
}

void miso_multiplex_end()
{
    MISO_INPUT;
    SPI_ENABLE;
}

void miso_multiplex_out(uint8_t value)
{
    MISO_OUTPUT;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (value & 1)
            MISO_HI;
        else
            MISO_LO;
        SCK_LO;
        _delay_us(2);
        SCK_HI;
        _delay_us(2);
        value >>= 1;
    }
    MISO_INPUT;
}

uint8_t miso_multiplex_in()
{
    uint8_t value = 0;
    MISO_INPUT;
    for (uint8_t i = 0; i < 8; i++)
    {
        value >>= 1;
        SCK_HI;
        _delay_us(2);
        SCK_LO;
        _delay_us(2);
        if (GET_MISO)
            value |= 0x80;
    }
    return value;
}