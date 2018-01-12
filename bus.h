#ifndef BUS_H
#define BUS_H

#include "defines.h"
#include <avr/io.h>

// CPU control /////////////////////////////////////////////////////////////////

#define HALT_INPUT CTRL2_DDR &= ~(1 << HALT)
#define HALT_PULLUP CTRL2_PORT |= (1 << HALT)
#define GET_HALT (CTRL2_PIN & (1 << HALT))

#define IOACK_OUTPUT CTRL_DDR |= (1 << IOACK)
#define GET_IOACK (CTRL_PIN & (1 << IOACK))
#define IOACK_HI CTRL_PORT |= (1 << IOACK)
#define IOACK_LO CTRL_PORT &= ~(1 << IOACK)

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

// CPU bus control /////////////////////////////////////////////////////////////

#define BUSRQ_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << BUSRQ))
#define GET_BUSRQ (iox_read(CTRLX_GPIO) & (1 << BUSRQ))
#define BUSRQ_LO iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << BUSRQ))
#define BUSRQ_HI iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << BUSRQ))

#define BUSACK_INPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) | (1 << BUSACK))
#define GET_BUSACK (iox_read(CTRLX_GPIO) & (1 << BUSACK))

// System control //////////////////////////////////////////////////////////////

#define CTRL_INPUT CTRL_DDR &= ~((1 << MREQ) | (1 << WR) | (1 << RD))
#define CTRL_OUTPUT CTRL_DDR |= ((1 << MREQ) | (1 << WR) | (1 << RD))
#define CTRL_HI CTRL_PORT |= ((1 << MREQ) | (1 << WR) | (1 << RD))

#ifdef M1
#define M1_INPUT CTRL2_DDR &= ~(1 << M1)
#define GET_M1 (CTRL2_PIN & (1 << M1))
#else
#define M1_INPUT
#define GET_M1 1
#endif

#define GET_MREQ (CTRL_PIN & (1 << MREQ))
#define MREQ_HI CTRL_PORT |= (1 << MREQ)
#define MREQ_LO CTRL_PORT &= ~(1 << MREQ)

#define IORQ_INPUT CTRL_DDR &= ~(1 << IORQ)
#define GET_IORQ (CTRL_PIN & (1 << IORQ))

#define GET_RD (CTRL_PIN & (1 << RD))
#define RD_HI CTRL_PORT |= (1 << RD)
#define RD_LO CTRL_PORT &= ~(1 << RD)

#define GET_WR (CTRL_PIN & (1 << WR))
#define WR_HI CTRL_PORT |= (1 << WR)
#define WR_LO CTRL_PORT &= ~(1 << WR)

#ifdef RFSH
#define RFSH_INPUT CTRL2_DDR &= ~(1 << RFSH)
#define GET_RFSH (CTRL2_PIN & (1 << RFSH))
#else
#define RFSH_INPUT
#define GET_RFSH 1
#endif

// Address bus /////////////////////////////////////////////////////////////////

#define ADDR_INPUT (ADDRLO_DDR = 0x00, iox_write(ADDRHI_IODIR, 0xFF))
#define ADDR_OUTPUT (ADDRLO_DDR = 0xFF, iox_write(ADDRHI_IODIR, 0x00))
#define GET_ADDRLO ADDRLO_PIN
#define SET_ADDRLO(addr) ADDRLO_PORT = (addr)
#define GET_ADDRHI iox_read(ADDRHI_GPIO)
#define SET_ADDRHI(addr) iox_write(ADDRHI_GPIO, (addr))
#define GET_ADDR (GET_ADDRLO | (GET_ADDRHI << 8))
#define SET_ADDR(addr) (SET_ADDRLO((addr) & 0xFF), SET_ADDRHI((addr) >> 8))

#define BANK_OUTPUT iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~BANKMASK)
#define GET_BANK ((iox_read(CTRLX_GPIO) & BANKMASK) >> BANKADDR)
#define SET_BANK(bank) iox_write(CTRLX_GPIO, (iox_read(CTRLX_GPIO) & ~BANKMASK) | (((bank) << BANKADDR) & BANKMASK))

// Data bus ///////////////////////////////////////////////////////////////////

#define DATA_INPUT DATA_DDR = 0x00
#define DATA_OUTPUT DATA_DDR = 0xFF
#define GET_DATA DATA_PIN
#define SET_DATA(data) DATA_PORT = (data)

// Clock ///////////////////////////////////////////////////////////////////////

#define CLK_OUTPUT CTRL_DDR |= (1 << CLK)
#define GET_CLK (CTRL_PIN & (1 << CLK))
#define CLK_HI CTRL_PORT |= (1 << CLK)
#define CLK_LO CTRL_PORT &= ~(1 << CLK)
#define CLK_TOGGLE CTRL_PIN |= (1 << CLK)

void clk_cycle(uint8_t cycles);
void clk_run(void);
void clk_stop(void);
void z80_reset(void);
void bus_master(void);
void bus_slave(void);
void bus_init(void);
void bus_status(void);
void bus_trace(uint16_t cycles);
void read_mem(uint16_t addr, uint8_t * buf, uint16_t len);
void write_mem(uint16_t addr, uint8_t * buf, uint16_t len);

#endif