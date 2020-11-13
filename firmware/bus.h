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
 * Address Bus
 */

#define ADDRLO_DDR DDRA
#define ADDRLO_PORT PORTA
#define ADDRLO_PIN PINA

#if (BOARD_REV == 3 || BOARD_REV == 4)
#define ADDRHI_IODIR IODIRA
#define ADDRHI_GPPU GPPUA
#define ADDRHI_GPIO GPIOA
#elif (BOARD_REV == 5 || BOARD_REV == 6)
#define ADDRHI_IODIR IODIRB
#define ADDRHI_GPPU GPPUB
#define ADDRHI_GPIO GPIOB
#else
#error "Unsupported board revision. Set BOARD_REV in your Makefile to match your board revision."
#endif

#define ADDR_INPUT (ADDRLO_DDR = 0x00, iox0_write(ADDRHI_IODIR, 0xFF))
#define ADDR_OUTPUT (ADDRLO_DDR = 0xFF, iox0_write(ADDRHI_IODIR, 0x00))

#define GET_ADDRLO ADDRLO_PIN
#define SET_ADDRLO(addr) ADDRLO_PORT = (addr)

#define GET_ADDRHI iox0_read(ADDRHI_GPIO)
#define SET_ADDRHI(addr) iox0_write(ADDRHI_GPIO, (addr))

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

#define BUSRQ (1 << 0)

#define GET_BUSRQ (PINB & BUSRQ)
#define BUSRQ_STATUS (status.flags & BUSRQ)
#define BUSRQ_HI PORTB |= BUSRQ
#define BUSRQ_LO PORTB &= ~BUSRQ

#define CTRLB_MASK 0x0F
#define CTRLB_OUTPUT_INIT DDRB |= BUSRQ

#if (BOARD_REV == 3 || BOARD_REV == 4)
#define IORQ (1 << 1)
#define MREQ (1 << 2)

#define GET_MREQ (PINB & MREQ)
#define MREQ_STATUS (status.flags & MREQ)
#define MREQ_HI PORTB |= MREQ
#define MREQ_LO PORTB &= ~MREQ

#define GET_IORQ (PINB & IORQ)
#define IORQ_STATUS (status.flags & IORQ)
#define IORQ_HI PORTB |= IORQ
#define IORQ_LO PORTB &= ~IORQ

#define IOMR_HI PORTB |= (IORQ | MREQ)
#define IOMR_OUTPUT DDRB |= (IORQ | MREQ)
#define IOMR_INPUT DDRB &= ~(IORQ | MREQ)

#define GET_WAIT (GET_IORQ || !GET_BUSRQ)
#define WAIT_STATUS (IORQ_STATUS || !BUSRQ_STATUS)

#define GET_IOXINT 0
#define IOXINT_STATUS 0
#elif (BOARD_REV == 5 || BOARD_REV == 6)
#define WAIT (1 << 1)
#define IOXINT (1 << 2)

#define GET_WAIT (PINB & WAIT)
#define WAIT_STATUS (status.flags & WAIT)

#define GET_IOXINT (PINB & IOXINT)
#define IOXINT_STATUS (status.flags & IOXINT)
#endif


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

/**
 * IOX flags
 */

#if (BOARD_REV == 3 || BOARD_REV == 4)
#define CTRLX_IODIR IODIRB
#define CTRLX_GPPU GPPUB
#define CTRLX_GPIO GPIOB
#define RFSH (1 << 1)
#define RESET (1 << 2)
#define INTERRUPT (1 << 3)
#define M1 (1 << 4)
#define HALT (1 << 5)
#define NMI (1 << 7)

#define CTRLX_OUTPUT_INIT // No-op

#define GET_RFSH (iox0_read(CTRLX_GPIO) & RFSH)
#define RFSH_STATUS (status.xflags & RFSH)

#elif (BOARD_REV == 5 || BOARD_REV == 6)
#define CTRLX_IODIR IODIRA
#define CTRLX_GPPU GPPUA
#define CTRLX_GPIO GPIOA
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
#endif

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

#define CTRLX_PULLUP_INIT iox0_set(CTRLX_GPPU, (HALT | RESET | INTERRUPT | NMI))

/**
 * Complete bus status all in one place
 */
typedef struct {
        uint8_t flags;
        uint8_t xflags;
        uint16_t addr;
        uint8_t data;
} bus_stat;

extern uint8_t clkdiv;
extern uint32_t base_addr;

void clk_cycle(uint8_t cycles);
void clk_run(void);
void clk_stop(void);

uint8_t bus_request(void);
void bus_release(void);
void bus_init(void);

bus_stat bus_status(void);
bus_stat bus_status_fast(void);
void bus_log(bus_stat status);

uint8_t io_out(uint8_t addr, uint8_t value);
uint8_t io_in(uint8_t addr);

#if defined(BANK_PORT) || defined(BANK_BASE)
void mem_bank(uint8_t bank, uint8_t page);
void mem_bank_addr(uint32_t addr);
#else
#define mem_bank(bank, page)
#define mem_bank_addr(addr)
#endif

typedef void (*pagefunc_t)(uint8_t, uint8_t, void*);
void mem_read_page(uint8_t start, uint8_t end, void *buf);
void mem_write_page(uint8_t start, uint8_t end, void *buf);
void mem_write_page_P(uint8_t start, uint8_t end, void *buf);
void mem_iterate(uint16_t start, uint16_t end, pagefunc_t dopage, void *buf);
void mem_iterate_banked(uint32_t start, uint32_t end, pagefunc_t dopage, void *buf);

#define mem_read(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_read_page, (buf));
#define mem_write(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_write_page, (buf));
#define mem_write_P(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_write_page_P, (void *)(buf));

#define mem_read_banked(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_read_page, (buf));
#define mem_write_banked(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_write_page, (buf));
#define mem_write_banked_P(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_write_page_P, (void *)(buf));

void sn76489_mute(void);

#endif