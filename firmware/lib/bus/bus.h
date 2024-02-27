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

#ifndef HARDWARE_BUS_H
#define HARDWARE_BUS_H

#include <avr/io.h>

#include "avr_gpio.h"
#include "mcp23s17.h"

/**
 * Address Bus
 */

#define ADDRLO_GPIO_PORT A
#define ADDRLO_INPUT GPIO_INPUT(ADDRLO_GPIO_PORT, GPIO_ALL)
#define ADDRLO_OUTPUT GPIO_OUTPUT(ADDRLO_GPIO_PORT, GPIO_ALL)
#define ADDRLO_READ GPIO_READ(ADDRLO_GPIO_PORT)
#define ADDRLO_WRITE(V) GPIO_WRITE(ADDRLO_GPIO_PORT, (V))

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

#define ADDRHI_INPUT iox0_write(ADDRHI_IODIR, 0xFF)
#define ADDRHI_OUTPUT iox0_write(ADDRHI_IODIR, 0x00)
#define ADDRHI_READ iox0_read(ADDRHI_GPIO)
#define ADDRHI_WRITE(addr) iox0_write(ADDRHI_GPIO, (addr))

#define ADDR_INPUT    \
    do {              \
        ADDRLO_INPUT; \
        ADDRHI_INPUT; \
    } while (0);
#define ADDR_OUTPUT    \
    do {               \
        ADDRLO_OUTPUT; \
        ADDRHI_OUTPUT; \
    } while (0);
#define ADDR_READ (ADDRLO_READ | (ADDRHI_READ << 8))
#define ADDR_WRITE(addr) (ADDRLO_WRITE((addr) & 0xFF), ADDRHI_WRITE((addr) >> 8))

/**
 * Data Bus
 */

#define DATA_GPIO_PORT C
#define DATA_INPUT GPIO_INPUT(DATA_GPIO_PORT, GPIO_ALL)
#define DATA_OUTPUT GPIO_OUTPUT(DATA_GPIO_PORT, GPIO_ALL)
#define DATA_READ GPIO_READ(DATA_GPIO_PORT)
#define DATA_WRITE(V) GPIO_WRITE(DATA_GPIO_PORT, (V))

/**
 * First group of control signals
 */

#define CTRL1_GPIO_PORT B
#define CTRL1_INPUT(V) GPIO_INPUT(CTRL1_GPIO_PORT, (V))
#define CTRL1_OUTPUT(V) GPIO_OUTPUT(CTRL1_GPIO_PORT, (V))
#define CTRL1_READ GPIO_READ(CTRL1_GPIO_PORT)
#define CTRL1_SET(V) GPIO_SET(CTRL1_GPIO_PORT, (V))
#define CTRL1_CLEAR(V) GPIO_CLEAR(CTRL1_GPIO_PORT, (V))

#define BUSRQ (1 << 0)

#define GET_BUSRQ (CTRL1_READ & BUSRQ)
#define BUSRQ_STATUS (status.flags & BUSRQ)
#define BUSRQ_HI CTRL1_SET(BUSRQ)
#define BUSRQ_LO CTRL1_CLEAR(BUSRQ)

#define CTRL1_MASK 0x0F
#define CTRL1_OUTPUT_INIT CTRL1_OUTPUT(BUSRQ)

#if (BOARD_REV == 3 || BOARD_REV == 4)
#define IORQ (1 << 1)
#define MREQ (1 << 2)

#define GET_MREQ (CTRL1_READ & MREQ)
#define MREQ_STATUS (status.flags & MREQ)
#define MREQ_HI CTRL1_SET(MREQ)
#define MREQ_LO CTRL1_CLEAR(MREQ)

#define GET_IORQ (CTRL1_READ & IORQ)
#define IORQ_STATUS (status.flags & IORQ)
#define IORQ_HI CTRL1_SET(IORQ)
#define IORQ_LO CTRL1_CLEAR(IORQ)

#define IOMR_HI CTRL1_SET((IORQ | MREQ))
#define IOMR_OUTPUT CTRL1_OUTPUT(IORQ | MREQ)
#define IOMR_INPUT CTRL1_INPUT(IORQ | MREQ)

#define GET_WAIT (GET_IORQ || !GET_BUSRQ)
#define WAIT_STATUS (IORQ_STATUS || !BUSRQ_STATUS)

#define GET_IOXINT 0
#define IOXINT_STATUS 0
#elif (BOARD_REV == 5 || BOARD_REV == 6)
#define WAIT (1 << 1)
#define IOXINT (1 << 2)

#define GET_WAIT (CTRL1_READ & WAIT)
#define WAIT_STATUS (status.flags & WAIT)

#define GET_IOXINT (CTRL1_READ & IOXINT)
#define IOXINT_STATUS (status.flags & IOXINT)
#endif

/**
 * Second group of control signals
 */

#define CTRL2_GPIO_PORT D
#define CTRL2_INPUT(V) GPIO_INPUT(CTRL2_GPIO_PORT, (V))
#define CTRL2_OUTPUT(V) GPIO_OUTPUT(CTRL2_GPIO_PORT, (V))
#define CTRL2_READ GPIO_READ(CTRL2_GPIO_PORT)
#define CTRL2_SET(V) GPIO_SET(CTRL2_GPIO_PORT, (V))
#define CTRL2_CLEAR(V) GPIO_CLEAR(CTRL2_GPIO_PORT, (V))
#define CTRL2_TOGGLE(V) GPIO_TOGGLE(CTRL2_GPIO_PORT, (V))

#define RD (1 << 4)
#define WR (1 << 5)
#define CLK (1 << 6)
#define BUSACK (1 << 7)

#define GET_RD (CTRL2_READ & RD)
#define RD_STATUS (status.flags & RD)
#define RD_HI CTRL2_SET(RD)
#define RD_LO CTRL2_CLEAR(RD)

#define GET_WR (CTRL2_READ & WR)
#define WR_STATUS (status.flags & WR)
#define WR_HI CTRL2_SET(WR)
#define WR_LO CTRL2_CLEAR(WR)

#define RDWR_INPUT CTRL2_INPUT(RD | WR)
#define RDWR_OUTPUT CTRL2_OUTPUT(RD | WR)
#define RDWR_HI CTRL2_SET((RD | WR))

#define GET_CLK (CTRL2_READ & CLK)
#define CLK_STATUS (status.flags & CLK)
#define CLK_HI CTRL2_SET(CLK)
#define CLK_LO CTRL2_CLEAR(CLK)
#define CLK_TOGGLE CTRL2_TOGGLE(CLK)

#define GET_BUSACK (CTRL2_READ & BUSACK)
#define BUSACK_STATUS (status.flags & BUSACK)

#define CTRL2_MASK 0xF0
#define CTRL2_OUTPUT_INIT CTRL2_OUTPUT(CLK)

/**
 * Third group of control signals
 */

#if (BOARD_REV == 3 || BOARD_REV == 4)
#define CTRLX_IODIR IODIRB
#define CTRLX_GPPU GPPUB
#define CTRLX_GPIO GPIOB
#define CTRLX_INTCON INTCONB
#define CTRLX_DEFVAL DEFVALB
#define CTRLX_GPINTEN GPINTENB
#define CTRLX_INTCAP INTCAPB
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

extern uint32_t base_addr;

void set_clkdiv(uint8_t div);
uint8_t get_clkdiv(void);
void clk_cycle(uint8_t cycles);
void clk_run(void);
void clk_stop(void);

uint8_t bus_request(void);
uint8_t bus_release(void);
void bus_init(void);

bus_stat bus_status(void);
bus_stat bus_status_fast(void);
void bus_log(bus_stat status);
#endif