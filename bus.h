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

#if (BOARD_REV < 3 || BOARD_REV > 4)
#error "Unsupported board revision. Set BOARD_REV in your Makefile to match your board revision."
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

#define ADDR_INPUT (ADDRLO_DDR = 0x00, iox_write(0, ADDRHI_IODIR, 0xFF))
#define ADDR_OUTPUT (ADDRLO_DDR = 0xFF, iox_write(0, ADDRHI_IODIR, 0x00))

#define GET_ADDRLO ADDRLO_PIN
#define SET_ADDRLO(addr) ADDRLO_PORT = (addr)

#define GET_ADDRHI iox_read(0, ADDRHI_GPIO)
#define SET_ADDRHI(addr) iox_write(0, ADDRHI_GPIO, (addr))

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

#define BUSRQ 0
#define IORQ 1
#define MREQ 2

#define IORQ_INPUT DDRB &= ~(1 << IORQ)
#define GET_IORQ (PINB & (1 << IORQ))

#define GET_BFLAGS (PINB & ((1<<BUSRQ) | (1<<IORQ) | (1<<MREQ)))

#define BUSRQ_OUTPUT DDRB |= (1 << BUSRQ)
#define GET_BUSRQ (PINB & (1 << BUSRQ))
#define BUSRQ_HI PORTB |= (1 << BUSRQ)
#define BUSRQ_LO PORTB &= ~(1 << BUSRQ)

#define MREQ_INPUT DDRB &= ~(1 << MREQ)
#define GET_MREQ (PINB & (1 << MREQ))
#define MREQ_OUTPUT DDRB |= (1 << MREQ)
#define MREQ_HI PORTB |= (1 << MREQ)
#define MREQ_LO PORTB &= ~(1 << MREQ)

#define IORQ_OUTPUT DDRB |= (1 << IORQ)
#define IORQ_HI PORTB |= (1 << IORQ)
#define IORQ_LO PORTB &= ~(1 << IORQ)

/**
 * PORTD Flags
 */

#define RD 4
#define WR 5
#define CLK 6
#define BUSACK 7

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

#define GET_DFLAGS (PIND & ((1<<RD) | (1<<WR) | (1<<CLK) | (1<<BUSACK)))

#define BUSACK_INPUT DDRD &= ~(1 << BUSACK)
#define GET_BUSACK (PIND & (1 << BUSACK))

/**
 * IOX GPIOB flags
 */

#define CTRLX_IODIR IODIRB0
#define CTRLX_GPPU GPPUB0
#define CTRLX_GPIO GPIOB0

#define RFSH 1
#define RESET 2
#define INTERRUPT 3
#define M1 4
#define HALT 5
#define NMI 7

#define RFSH_INPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) | (1 << RFSH))
#define GET_RFSH (iox_read(0, CTRLX_GPIO) & (1 << RFSH))

// Reset behaves like an open drain. It is only an output when it is low.
#define RESET_INPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) | (1 << RESET))
#define RESET_PULLUP iox_write(0, CTRLX_GPPU, iox_read(0, CTRLX_GPPU) | (1 << RESET))
#define GET_RESET (iox_read(0, CTRLX_GPIO) & (1 << RESET))
#define RESET_LO (iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) & ~(1 << RESET)), iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) & ~(1 << RESET)))
#define RESET_HI (iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) | (1 << RESET)), iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) | (1 << RESET)))

#define INT_OUTPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) & ~(1 << INTERRUPT))
#define GET_INT (iox_read(0, CTRLX_GPIO) & (1 << INTERRUPT))
#define INT_LO iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) & ~(1 << INTERRUPT))
#define INT_HI iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) | (1 << INTERRUPT))

#define GET_XFLAGS (iox_read(0, CTRLX_GPIO) & ((1<<RFSH) | (1<<RESET) | (1<<INTERRUPT) | \
                                        (1<<M1) | (1<<HALT) | (1<<NMI)))

#define M1_INPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) | (1 << M1))
#define GET_M1 (iox_read(0, CTRLX_GPIO) & (1 << M1))

#define HALT_INPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) | (1 << HALT))
#define HALT_PULLUP iox_write(0, CTRLX_GPPU, iox_read(0, CTRLX_GPPU) | (1 << HALT))
#define GET_HALT (iox_read(0, CTRLX_GPIO) & (1 << HALT))

#define NMI_OUTPUT iox_write(0, CTRLX_IODIR, iox_read(0, CTRLX_IODIR) & ~(1 << NMI))
#define GET_NMI (iox_read(0, CTRLX_GPIO) & (1 << NMI))
#define NMI_LO iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) & ~(1 << NMI))
#define NMI_HI iox_write(0, CTRLX_GPIO, iox_read(0, CTRLX_GPIO) | (1 << NMI))

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
extern uint32_t base_addr;

void clk_cycle(uint8_t cycles);
void clk_run(void);
void clk_stop(void);
uint8_t bus_master(void);
void bus_slave(void);
bus_stat bus_status(void);
void bus_log(bus_stat status);
void bus_init(void);

void mem_read_bare(uint32_t addr, uint8_t * buf, uint16_t len);
void mem_read(uint32_t addr, uint8_t * buf, uint16_t len);
void _mem_write_bare(uint32_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace);
void _mem_write(uint32_t addr, const uint8_t *buf, uint16_t len, uint8_t pgmspace);

#define mem_write(addr, buf, len) _mem_write((addr), (buf), (len), 0);
#define mem_write_P(addr, buf, len) _mem_write((addr), (buf), (len), 1);

#define mem_write_bare(addr, buf, len) _mem_write_bare((addr), (buf), (len), 0);
#define mem_write_bare_P(addr, buf, len) _mem_write_bare((addr), (buf), (len), 1);

void io_out_bare(uint8_t addr, uint8_t value);
void io_out(uint8_t addr, uint8_t value);
uint8_t io_in_bare(uint8_t addr);
uint8_t io_in(uint8_t addr);

void sn76489_mute(void);

#ifdef PAGE_BASE
#define PAGE(addr) ((addr) >> 14)
void mem_page_bare(uint8_t bank, uint8_t page);
void mem_page(uint8_t bank, uint8_t page);
#endif

#endif