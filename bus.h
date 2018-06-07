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

/**
 * @file bus.h Low-level control of the Z80 bus
 */

#ifndef BUS_H
#define BUS_H

#include <avr/io.h>
#include "iox.h"

/**
 * Ensure board revision is set to a supported value
 */

#if ((BOARD_REV < 1 || BOARD_REV > 3) && BOARD_REV != -3)
#error "Unsupported board revision. Set BOARD_REV in your Makefile to match your board revision."
#endif

/**
 * Enable outbound IORQ on board revisions where it's safe
 */

#if (BOARD_REV == -3 || BOARD_REV >= 3)
#define OUTBOUND_IORQ
#endif

/**
 * Address Bus
 */

#define ADDRLO_DDR DDRA
#define ADDRLO_PORT PORTA
#define ADDRLO_PIN PINA

#define ADDRHI_IODIR IODIRA0
#define ADDRHI_GPPU GPPUA0
#define ADDRHI_GPIO GPIOA0

#define ADDR_INPUT (ADDRLO_DDR = 0x00, iox_write(ADDRHI_IODIR, 0xFF))
#define ADDR_OUTPUT (ADDRLO_DDR = 0xFF, iox_write(ADDRHI_IODIR, 0x00))

#define GET_ADDRLO ADDRLO_PIN
#define SET_ADDRLO(addr) ADDRLO_PORT = (addr)

#define GET_ADDRHI iox_read(ADDRHI_GPIO)
#define SET_ADDRHI(addr) iox_write(ADDRHI_GPIO, (addr))

#define GET_ADDR (GET_ADDRLO | (GET_ADDRHI << 8))
#define SET_ADDR(addr) (SET_ADDRLO((addr) & 0xFF), SET_ADDRHI((addr) >> 8))

/**
 * Data Bus
 */

#define DATA_DDR DDRC
#define DATA_PORT PORTC
#define DATA_PIN PINC

#define DATA_INPUT DATA_DDR = 0x00
#define DATA_OUTPUT DATA_DDR = 0xFF
#define GET_DATA DATA_PIN
#define SET_DATA(data) DATA_PORT = (data)

/**
 * PORTB flags
 */

#define IOACK 0
#define IORQ 1
#if (BOARD_REV == 1 || BOARD_REV == 2)
#define M1 2
#else
#define BUSRQ 2
#endif

#if (BOARD_REV == 1 || BOARD_REV == 2)
#define GET_BFLAGS (PINB & ((1<<IOACK) | (1<<IORQ) | (1<<M1)))
#else
#define GET_BFLAGS (PINB & ((1<<IOACK) | (1<<IORQ) | (1<<BUSRQ)))
#endif

#define IOACK_OUTPUT DDRB |= (1 << IOACK)
#define GET_IOACK (PINB & (1 << IOACK))
#define IOACK_HI PORTB |= (1 << IOACK)
#define IOACK_LO PORTB &= ~(1 << IOACK)

#define IORQ_INPUT DDRB &= ~(1 << IORQ)
#define GET_IORQ (PINB & (1 << IORQ))
#ifdef OUTBOUND_IORQ
#define IORQ_OUTPUT DDRB |= (1 << IORQ)
#define IORQ_HI PORTB |= (1 << IORQ)
#define IORQ_LO PORTB &= ~(1 << IORQ)
#endif

#if (BOARD_REV == 1 || BOARD_REV == 2)
#define M1_INPUT DDRB &= ~(1 << M1)
#define GET_M1 (PINB & (1 << M1))
#else
#define BUSRQ_OUTPUT DDRB |= (1 << BUSRQ)
#define GET_BUSRQ (PINB & (1 << BUSRQ))
#define BUSRQ_HI PORTB |= (1 << BUSRQ)
#define BUSRQ_LO PORTB &= ~(1 << BUSRQ)
#endif

/**
 * PORTD Flags
 */

#define RD 4
#define WR 5
#define CLK 6
#define HALT 7

#define GET_DFLAGS (PIND & ((1<<RD) | (1<<WR) | (1<<CLK) | (1<<HALT)))

#define RD_INPUT DDRD &= ~(1 << RD)
#define RD_OUTPUT DDRD |= (1 << RD)
#define GET_RD (PIND & (1 << RD))
#define RD_HI PORTD |= (1 << RD)
#define RD_LO PORTD &= ~(1 << RD)

#define WR_INPUT DDRD &= ~(1 << WR)
#define WR_OUTPUT DDRD |= (1 << WR)
#define GET_WR (PIND & (1 << WR))
#define WR_HI PORTD |= (1 << WR)
#define WR_LO PORTD &= ~(1 << WR)

#define CLK_OUTPUT DDRD |= (1 << CLK)
#define GET_CLK (PIND & (1 << CLK))
#define CLK_HI PORTD |= (1 << CLK)
#define CLK_LO PORTD &= ~(1 << CLK)
#define CLK_TOGGLE PIND |= (1 << CLK)

#define HALT_INPUT DDRD &= ~(1 << HALT)
#define HALT_PULLUP PORTD |= (1 << HALT)
#define GET_HALT (PIND & (1 << HALT))

/**
 * IOX GPIOB flags
 */

#define CTRLX_IODIR IODIRB0
#define CTRLX_GPPU GPPUB0
#define CTRLX_GPIO GPIOB0

#if (BOARD_REV == 1 || BOARD_REV == 2)
#define RFSH 1
#define BUSRQ 6
#else
#define M1 1
#endif
#define RESET 2
#define INTERRUPT 3
#define BUSACK 4
#define MREQ 5
#define NMI 7

#if (BOARD_REV == 1 || BOARD_REV == 2)
#define GET_XFLAGS (iox_read(CTRLX_GPIO) & ((1<<RFSH) | (1<<RESET) | (1<<INTERRUPT) | \
                                        (1<<BUSACK) | (1<<MREQ) | (1<<BUSRQ) | (1<<NMI)))
#else
#define GET_XFLAGS (iox_read(CTRLX_GPIO) & ((1<<M1) | (1<<RESET) | (1<<INTERRUPT) | \
                                        (1<<BUSACK) | (1<<MREQ) | (1<<NMI)))
#endif

#define INT_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << INTERRUPT))
#define GET_INT (iox_read(CTRLX_GPIO) & (1 << INTERRUPT))
#define INT_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << INTERRUPT))
#define INT_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << INTERRUPT))

#define NMI_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << NMI))
#define GET_NMI (iox_read(CTRLX_GPIO) & (1 << NMI))
#define NMI_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << NMI))
#define NMI_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << NMI))

#define RESET_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << RESET))
#define GET_RESET (iox_read(CTRLX_GPIO) & (1 << RESET))
#define RESET_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << RESET))
#define RESET_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << RESET))

#if (BOARD_REV == 1 || BOARD_REV == 2)
#define BUSRQ_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << BUSRQ))
#define GET_BUSRQ (iox_read(CTRLX_GPIO) & (1 << BUSRQ))
#define BUSRQ_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << BUSRQ))
#define BUSRQ_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << BUSRQ))
#else
#define M1_INPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) | (1 << M1))
#define GET_M1 (iox_read(CTRLX_GPIO) & (1 << M1))
#endif

#define BUSACK_INPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) | (1 << BUSACK))
#define GET_BUSACK (iox_read(CTRLX_GPIO) & (1 << BUSACK))

#define MREQ_INPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) | (1 << MREQ))
#define MREQ_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << MREQ))
#define GET_MREQ (iox_read(CTRLX_GPIO) & (1 << MREQ))
#define MREQ_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << MREQ))
#define MREQ_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << MREQ))

/**
 * Complete bus status all in one place
 */
typedef struct {
        uint8_t flags;
        uint8_t xflags;
        uint16_t addr;
        uint8_t data;
} bus_stat;

#define FLAG(b, f) ((b) & (1<<f))

extern uint8_t clkdiv;

void clk_cycle(uint8_t cycles);
void clk_run(void);
void clk_stop(void);
uint8_t bus_master(void);
void bus_slave(void);
bus_stat bus_status(void);
void bus_init(void);

void read_mem(uint16_t addr, uint8_t * buf, uint16_t len);
void _write_mem(uint16_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace);

#define write_mem(addr, buf, len) _write_mem((addr), (buf), (len), 0);
#define write_mem_P(addr, buf, len) _write_mem((addr), (buf), (len), 1);

#ifdef OUTBOUND_IORQ
void io_out(uint8_t addr, uint8_t value);
uint8_t io_in(uint8_t addr);
void mem_page(uint8_t bank, uint8_t page);
void mem_restore(void);
#endif

#endif