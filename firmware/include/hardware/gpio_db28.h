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
 * @file bus.h Low-level control of the Z80 bus
 */

#ifndef HARDWARE_GPIO_DB28_H
#define HARDWARE_GPIO_DB28_H

#include <avr/io.h>

#include "mcp23s17.h"

/**
 * Address Bus
 */

#if (BOARD_REV < 10)
#error "Unsupported board revision. Set BOARD_REV in your Makefile to match your board revision."
#endif

#define ADDRHI_IODIR IODIRA
#define ADDRHI_GPPU GPPUA
#define ADDRHI_GPIO GPIOA

#define ADDRLO_IODIR IODIRB
#define ADDRLO_GPPU GPPUB
#define ADDRLO_GPIO GPIOB

#define ADDR_INPUT (iox0_write(ADDRLO_IODIR, 0xFF), iox0_write(ADDRHI_IODIR, 0xFF))
#define ADDR_OUTPUT (iox0_write(ADDRLO_IODIR, 0x00), iox0_write(ADDRHI_IODIR, 0x00))

#define GET_ADDRLO iox0_read(ADDRLO_GPIO)
#define SET_ADDRLO(addr) iox0_write(ADDRLO_GPIO, (addr))

#define GET_ADDRHI iox0_read(ADDRHI_GPIO)
#define SET_ADDRHI(addr) iox0_write(ADDRHI_GPIO, (addr))

#define GET_ADDR (GET_ADDRLO | (GET_ADDRHI << 8))
#define SET_ADDR(addr) (SET_ADDRLO((addr) & 0xFF), SET_ADDRHI((addr) >> 8))

/**
 * Data Bus
 */

#define DATA_IODIR IODIRA
#define DATA_GPPU GPPUA
#define DATA_GPIO GPIOA

#define DATA_INPUT iox_write(1, DATA_IODIR, 0xFF)
#define DATA_OUTPUT iox_write(1, DATA_IODIR, 0x00)
#define GET_DATA iox_read(1, DATA_GPIO)
#define SET_DATA(data) iox_write(1, DATA_GPIO, (data))

/**
 * PORTA flags
 */

#define BUSRQ (1 << 0)

#define GET_BUSRQ (PINB & BUSRQ)
#define BUSRQ_STATUS (status.flags & BUSRQ)
#define BUSRQ_HI PORTB |= BUSRQ
#define BUSRQ_LO PORTB &= ~BUSRQ

#define CTRLA_MASK 0x0F
#define CTRLA_OUTPUT_INIT DDRB |= BUSRQ

#define WAIT (1 << 1)
#define IOXINT (1 << 2)

#define GET_WAIT (PINB & WAIT)
#define WAIT_STATUS (status.flags & WAIT)

#define GET_IOXINT (PINB & IOXINT)
#define IOXINT_STATUS (status.flags & IOXINT)

/**
 * PORTD Flags
 */

#define RD (1 << 4)
#define WR (1 << 5)
#define CLK (1 << 6)
#define BUSACK (1 << 7)

#define GET_RD (PIND & RD)
#define RD_STATUS (status.flags & RD)
#define RD_HI PORTD |= RD
#define RD_LO PORTD &= ~RD

#define GET_WR (PIND & WR)
#define WR_STATUS (status.flags & WR)
#define WR_HI PORTD |= WR
#define WR_LO PORTD &= ~WR

#define RDWR_INPUT DDRD &= ~(RD | WR)
#define RDWR_OUTPUT DDRD |= (RD | WR)
#define RDWR_HI PORTD |= (RD | WR)

#define GET_CLK (PIND & CLK)
#define CLK_STATUS (status.flags & CLK)
#define CLK_HI PORTD |= CLK
#define CLK_LO PORTD &= ~CLK
#define CLK_TOGGLE PIND |= CLK

#define GET_BUSACK (PIND & BUSACK)
#define BUSACK_STATUS (status.flags & BUSACK)

#define CTRLD_MASK 0xF0
#define CTRLD_OUTPUT_INIT DDRD |= CLK

#define CTRLX_IODIR IODIRA
#define CTRLX_GPPU GPPUA
#define CTRLX_GPIO GPIOA
#define CTRLX_INTCON INTCONA
#define CTRLX_DEFVAL DEFVALA
#define CTRLX_GPINTEN GPINTENA
#define CTRLX_INTCAP INTCAPA
#define M1 (1 << 0)
#define RESET (1 << 1)
#define INTERRUPT (1 << 2)
#define IORQ (1 << 3)
#define NMI (1 << 4)
#define HALT (1 << 5)
#define MREQ (1 << 6)
#define MWAIT (1 << 7)

#define CTRLX_OUTPUT_INIT iox0_clear(CTRLX_IODIR, MWAIT)

#define GET_MWAIT (iox0_read(CTRLX_GPIO) & MWAIT)
#define MWAIT_STATUS (status.xflags & MWAIT)
#define MWAIT_LO iox0_clear(CTRLX_GPIO, MWAIT)
#define MWAIT_HI iox0_set(CTRLX_GPIO, MWAIT)

#define GET_MREQ (iox0_read(CTRLX_GPIO) & MREQ)
#define MREQ_STATUS (status.xflags & MREQ)
#define MREQ_LO iox0_clear(CTRLX_GPIO, MREQ)
#define MREQ_HI iox0_set(CTRLX_GPIO, MREQ)

#define GET_IORQ (iox0_read(CTRLX_GPIO) & IORQ)
#define IORQ_STATUS (status.xflags & IORQ)
#define IORQ_LO iox0_clear(CTRLX_GPIO, IORQ)
#define IORQ_HI iox0_set(CTRLX_GPIO, IORQ)

#define IOMR_INPUT iox0_set(CTRLX_IODIR, IORQ | MREQ)
#define IOMR_OUTPUT iox0_clear(CTRLX_IODIR, IORQ | MREQ)
#define IOMR_HI iox0_set(CTRLX_GPIO, IORQ | MREQ)

#define GET_RFSH (GET_MREQ || !(GET_RD && GET_WR))
#define RFSH_STATUS (MREQ_STATUS || !(RD_STATUS && WR_STATUS))

#define GET_M1 (iox0_read(CTRLX_GPIO) & M1)
#define M1_STATUS (status.xflags & M1)

#define GET_HALT (iox0_read(CTRLX_GPIO) & HALT)
#define HALT_STATUS (status.xflags & HALT)

// RESET, INT, and NMI behave like an open drain. It is only an output when it is low.
#define GET_RESET (iox0_read(CTRLX_GPIO) & RESET)
#define RESET_STATUS (status.xflags & RESET)
#define RESET_OUTPUT iox0_clear(CTRLX_IODIR, RESET)
#define RESET_INPUT iox0_set(CTRLX_IODIR, RESET)

#define GET_INT (iox0_read(CTRLX_GPIO) & INTERRUPT)
#define INT_STATUS (status.xflags & INTERRUPT)
#define INT_OUTPUT iox0_clear(CTRLX_IODIR, INTERRUPT)
#define INT_INPUT iox0_set(CTRLX_IODIR, INTERRUPT)

#define GET_NMI (iox0_read(CTRLX_GPIO) & NMI)
#define NMI_STATUS (status.xflags & NMI)
#define NMI_OUTPUT iox0_clear(CTRLX_IODIR, NMI)
#define NMI_INPUT iox0_set(CTRLX_IODIR, NMI)

#define IOX_CS_PORT PORTD
#define IOX_CS_PIN PIN7_bm
#define SD_CS_PORT PORTA
#define SD_CS_PIN PIN7_bm

#define IOX_CS IOX_CS_PORT.OUTCLR = IOX_CS_PIN
#define SD_CS SD_CS_PORT.OUTCLR = SD_CS_PIN
#define IDLE_CS (IOX_CS_PORT.OUTSET = IOX_CS_PIN, IOX_CS_PORT.OUTSET = SD_CS_PIN)

#define CHIP_SELECT(ADDR) if (ADDR == IOX_ADDR) { SD_CS_HI; IOX_CS_LO; } else if (ADDR == SD_ADDR) { IOX_CS_HI; SD_CS_LO; } else { IOX_CS_HI; SD_CS_HI; }

#endif