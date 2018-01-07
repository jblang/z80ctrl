#include "defines.h"

#include "bus.h"
#include "iox.h"

#include <stdio.h>
#include <util/delay.h>

// CPU control /////////////////////////////////////////////////////////////////

inline void ctrlx_pullup(void)
{
    iox_write(CTRLX_GPPU, 0xFF);
}

#ifdef HALT
inline void halt_input(void)
{
    CTRL2_PIN &= ~(1 << HALT);
}

inline uint8_t get_halt(void)
{
    return CTRL2_PIN & (1 << HALT);
}
#endif

inline void ioack_output(void)
{
    CTRL_DDR |= (1 << IOACK);
}

inline uint8_t get_ioack(void)
{
    return CTRL_PIN & (1 << IOACK);
}

inline void ioack_hi(void)
{
    CTRL_PORT |= (1 << IOACK);
}

inline void ioack_lo(void)
{
    CTRL_PORT &= ~(1 << IOACK);
}

inline void int_output(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << INT));
}

inline uint8_t get_int(void)
{
    return !!(iox_read(CTRLX_GPIO) & (1 << INT));
}

inline void int_lo(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << INT));
}

inline void int_hi(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << INT));
}

inline void nmi_output(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << NMI));
}

inline uint8_t get_nmi(void)
{
    return iox_read(CTRLX_GPIO) & (1 << NMI);
}

inline void nmi_lo(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << NMI));
}

inline void nmi_hi(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << NMI));
}

inline void reset_output(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << RESET));
}

inline uint8_t get_reset()
{
    return iox_read(CTRLX_GPIO) & (1 << RESET);
}

inline void reset_lo(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << RESET));
}

inline void reset_hi(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << RESET));
}

// CPU bus control /////////////////////////////////////////////////////////////

inline void busrq_output(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~(1 << BUSRQ));
}

inline uint8_t get_busrq(void)
{
    return iox_read(CTRLX_GPIO) & (1 << BUSRQ);
}

inline void busrq_lo(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) & ~(1 << BUSRQ));
}

inline void busrq_hi(void)
{
    iox_write(CTRLX_GPIO, iox_read(CTRLX_GPIO) | (1 << BUSRQ));
}

inline void busack_input(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) | (1 << BUSACK));
}

inline uint8_t get_busack(void)
{
    return iox_read(CTRLX_GPIO) & (1 << BUSACK);
}

// System control //////////////////////////////////////////////////////////////

inline void ctrl_input(void)
{
    CTRL_PORT |= ((1 << IORQ) | (1 << MREQ) | (1 << WR) | (1 << RD));
    CTRL_DDR &= ~((1 << IORQ) | (1 << MREQ) | (1 << WR) | (1 << RD));
}

inline void ctrl_output(void)
{
    CTRL_PORT |= ((1 << IORQ) | (1 << MREQ) | (1 << WR) | (1 << RD));
    CTRL_DDR |= ((1 << IORQ) | (1 << MREQ) | (1 << WR) | (1 << RD));
}

#ifdef M1
inline void m1_input(void)
{
    CTRL2_DDR &= ~(1 << M1);
}

inline uint8_t get_m1(void)
{
    return CTRL2_PIN & (1 << M1);
}
#endif

inline uint8_t get_mreq(void)
{
    return CTRL_PIN & (1 << MREQ);
}

inline void mreq_hi(void)
{
    CTRL_PORT |= (1 << MREQ);
}

inline void mreq_lo(void)
{
    CTRL_PORT &= ~(1 << MREQ);
}

inline uint8_t get_iorq(void)
{
    return CTRL_PIN & (1 << IORQ);
}

inline void iorq_hi(void)
{
    CTRL_PORT |= (1 << IORQ);
}

inline void iorq_lo(void)
{
    CTRL_PORT &= ~(1 << IORQ);
}

inline uint8_t get_rd(void)
{
    return CTRL_PIN & (1 << RD);
}

inline void rd_hi(void)
{
    CTRL_PORT |= (1 << RD);
}

inline void rd_lo(void)
{
    CTRL_PORT &= ~(1 << RD);
}

inline uint8_t get_wr(void)
{
    return CTRL_PIN & (1 << WR);
}

inline void wr_hi(void)
{
    CTRL_PORT |= (1 << WR);
}

inline void wr_lo(void)
{
    CTRL_PORT &= ~(1 << WR);
}

#ifdef RFSH
inline void rfsh_input(void)
{
    CTRL2_DDR &= ~(1 << RFSH);
}

inline uint8_t get_rfsh(void)
{
    return CTRL2_PIN & (1 << RFSH);
}
#endif

// Address bus /////////////////////////////////////////////////////////////////

inline void addrhi_pullup(void)
{
    iox_write(ADDRHI_GPPU, 0xFF);
}

inline void addr_input(void)
{
    ADDRLO_DDR = 0x00;
    iox_write(ADDRHI_IODIR, 0xFF);
}

inline void addr_output(void)
{
    ADDRLO_DDR = 0xFF;
    iox_write(ADDRHI_IODIR, 0x00);
}

inline uint8_t get_addrlo(void)
{
    return ADDRLO_PIN;
}

inline void set_addrlo(uint8_t addr)
{
    ADDRLO_PORT = addr;
}

inline uint8_t get_addrhi(void)
{
    return iox_read(ADDRHI_GPIO);
}

inline void set_addrhi(uint8_t addr)
{
    iox_write(ADDRHI_GPIO, addr);
}

inline uint16_t get_addr(void)
{
    return get_addrlo() | (get_addrhi() << 8);
}

inline void set_addr(uint16_t addr)
{
    set_addrlo(addr & 0xFF);
    set_addrhi(addr >> 8);
}

inline void bank_output(void)
{
    iox_write(CTRLX_IODIR, iox_read(CTRLX_IODIR) & ~BANKMASK);
}

inline uint8_t get_bank(void)
{
    return (iox_read(CTRLX_GPIO) & BANKMASK) >> BANKADDR;
}

inline void set_bank(uint8_t bank)
{
    iox_write(CTRLX_GPIO, (iox_read(CTRLX_GPIO) & ~BANKMASK) | ((bank << BANKADDR) & BANKMASK));
}

// Data bus ///////////////////////////////////////////////////////////////////

inline void data_input(void)
{
    DATA_DDR = 0x00;
}

inline void data_output(void)
{
    DATA_DDR = 0xFF;
}

inline uint8_t get_data(void)
{
    return DATA_PIN;
}

inline void set_data(uint8_t data)
{
    DATA_PORT = data;
}

// Clock ///////////////////////////////////////////////////////////////////////

inline void clk_output(void)
{
    CTRL_DDR |= (1 << CLK);
}

inline uint8_t get_clk(void)
{
    return CTRL_PIN & (1 << CLK);
}

inline void clk_hi(void)
{
    CTRL_PORT |= (1 << CLK);
}

inline void clk_lo(void)
{
    CTRL_PORT &= ~(1 << CLK);
}

inline void clk_tick(void)
{
    clk_hi();
#ifdef DEBUG
    bus_status();
#endif
    clk_lo();
#ifdef DEBUG
    bus_status();
#endif
}

inline void clk_run(void)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2A0) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = 200;
    OCR2B = 100;
}

inline void clk_stop()
{
    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = 0;
    OCR2B = 0;
}

// Convenience functions //////////////////////////////////////////////////////

void bus_master(void)
{
    busrq_lo();
    while (!get_busack());
    ctrl_output();
    addr_output();
    data_input();
}

void bus_slave(void)
{
    ctrl_input();
    addr_input();
    data_input();
    busrq_hi();
}

void bus_init(void)
{
    ctrlx_pullup();
    addrhi_pullup();

    int_output();
    nmi_output();
    reset_output();
    busrq_output();
    busack_input();
    bank_output();

    int_hi();
    nmi_hi();
    reset_lo();
    busrq_hi();
    set_bank(0);

#ifdef M1
    m1_input();
#endif
#ifdef RFSH
    rfsh_input();
#endif
#ifdef HALT
    halt_input();
#endif
    ioack_output();
    ioack_lo();
    ioack_hi();

    clk_output();
    clk_lo();
    clk_tick();
    clk_tick();
    clk_tick();

    bus_slave();
}

void bus_status(void)
{
    printf(
#ifdef M1
        "m1=%x "
#endif        
        "mreq=%x iorq=%x rd=%x wr=%x "
#ifdef RFSH
        "rfsh=%x "
#endif
#ifdef HALT
        "halt=%x "
#endif
        "wait=%x "
        "int=%x nmi=%x reset=%x busrq=%x busack=%x "
        "clk=%x bank=%03x addr=%04x data=%02x\n",
#ifdef M1
        !!get_m1(), 
#endif
        !!get_mreq(), !!get_iorq(), !!get_rd(), !!get_wr(),
#ifdef RFSH
        !!get_rfsh(), 
#endif
#ifdef HALT
        !!get_halt(), 
#endif
        !!get_ioack(),
        !!get_int(), !!get_nmi(), !!get_reset(), !!get_busrq(), !!get_busack(),
        !!get_clk(), get_bank(), get_addr(), get_data());
}

void read_mem(uint16_t addr, uint8_t * buf, uint16_t len)
{
    data_input();
    mreq_lo();
    rd_lo();
    set_addr(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        buf[i] = get_data();
        addr++;
        if (addr & 0xFF == 0)
            set_addr(addr);
        else
            set_addrlo(addr);
    }
    rd_hi();
    mreq_hi();
}

void write_mem(uint16_t addr, uint8_t * buf, uint16_t len)
{
    data_output();
    mreq_lo();
    set_addr(addr);
    uint16_t i;
    for (i = 0; i < len; i++) {
        set_data(buf[i]);
        wr_lo();
        wr_hi();
        addr++;
        if (addr & 0xFF == 0)
            set_addr(addr);
        else
            set_addrlo(addr & 0xFF);
    }
    mreq_hi();
}
