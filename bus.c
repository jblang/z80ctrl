#include "defines.h"

#include "bus.h"
#include "iox.h"

#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>

// CPU control /////////////////////////////////////////////////////////////////

inline void halt_input(void)
{
#ifdef HALT
    CTRL2_PIN &= ~(1 << HALT);
#endif
}

inline uint8_t get_halt(void)
{
#ifdef HALT
    return CTRL2_PIN & (1 << HALT);
#else
    return 1;
#endif
}

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
    CTRL_PORT |= ((1 << MREQ) | (1 << WR) | (1 << RD));
    CTRL_DDR &= ~((1 << MREQ) | (1 << WR) | (1 << RD));
}

inline void ctrl_output(void)
{
    CTRL_PORT |= ((1 << MREQ) | (1 << WR) | (1 << RD));
    CTRL_DDR |= ((1 << MREQ) | (1 << WR) | (1 << RD));
}

inline void m1_input(void)
{
#ifdef M1
    CTRL2_DDR &= ~(1 << M1);
#endif
}

inline uint8_t get_m1(void)
{
#ifdef M1
    return CTRL2_PIN & (1 << M1);
#else
    return 1;
#endif
}

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

inline void iorq_input(void)
{
    CTRL_DDR ^= ~ (1 << IORQ);
}

inline uint8_t get_iorq(void)
{
    return CTRL_PIN & (1 << IORQ);
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

inline void rfsh_input(void)
{
#ifdef RFSH
    CTRL2_DDR &= ~(1 << RFSH);
#endif
}

inline uint8_t get_rfsh(void)
{
#ifdef RFSH
    return CTRL2_PIN & (1 << RFSH);
#else
    return 1;
#endif
}

// Address bus /////////////////////////////////////////////////////////////////

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

inline void clk_toggle(void)
{
    CTRL_PORT ^= (1 << CLK);
}

// Run the Z80's clock
inline void clk_run(void)
{
    // Fast PWM mode with adjustable top and no prescaler
    TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << WGM22) | (1 << CS20);
    OCR2A = 90;
    OCR2B = 45;
}

// Stop the Z80's clock
inline void clk_stop()
{
    TCCR2A = 0;
    TCCR2B = 0;
    OCR2A = 0;
    OCR2B = 0;
}

// IORQ interrupt handler //////////////////////////////////////////////////////

void enable_iorq_int(void)
{
    // trigger interrupt on falling edge of IORQ
    EICRA = (1 << ISC01);
    EIMSK = (1 << INT0);
    sei();
}

volatile int int0cnt = 0;

ISR(INT0_vect)
{
    //if(get_addrlo() == 0)
        putchar(get_data());
    ioack_lo();
    _delay_us(10);
    ioack_hi();
}

// Convenience functions //////////////////////////////////////////////////////

// Request control of the bus from the Z80
void bus_master(void)
{
    busrq_lo();
    while (!get_busack())
        ;
    ctrl_output();
    addr_output();
    data_input();
}

// Return control of the bus to the Z80
void bus_slave(void)
{
    ctrl_input();
    addr_input();
    data_input();
    busrq_hi();
}

// Initialize bus
void bus_init(void)
{
    // Initialize I/O expander
    iox_init();

    // Configure bus signal direction
    int_output();
    nmi_output();
    reset_output();
    busrq_output();
    bank_output();
    iorq_input();
    ioack_output();
    clk_output();
    busack_input();
    m1_input();
    rfsh_input();
    halt_input();

    // Set defaults for output bus signals
    int_hi();
    nmi_hi();
    reset_lo();
    busrq_hi();
    clk_lo();
    set_bank(0);

    // Setup IORQ handling
    enable_iorq_int();
    ioack_lo();
    ioack_hi();

    // Make bidirectional signals inputs
    bus_slave();
}

// Log current bus status
void bus_status(void)
{
    uint8_t data = get_data();
    uint8_t ascii = (data >= 0x20 && data <= 0x7e) ? data : ' ';

    printf(
#ifdef M1
        "m1=%x "
#else
        "m1=X "
#endif        
        "mreq=%x iorq=%x rd=%x wr=%x "
#ifdef RFSH
        "rfsh=%x "
#else
        "rfsh=X "
#endif
#ifdef HALT
        "halt=%x "
#else
        "halt=X "
#endif
        "wait=%x "
        "int=%x nmi=%x reset=%x busrq=%x busack=%x "
        "clk=%x bank=%x addr=%04x data=%02x %c\n",
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
        !!get_clk(), get_bank(), get_addr(), data, ascii);
}

// Trace the bus state for specified number of clock cycles
void bus_trace(uint16_t cycles)
{
    clk_stop();
    clk_lo();
    uint16_t i;
    for (i = 0; i < cycles && get_halt() != 0; i++) {
        clk_hi();
        printf("%05d: ", i);
        bus_status();
        clk_lo();
        printf("%05d: ", i);
        bus_status();
    }
}

// Read specified number of bytes from external memory to a buffer
void read_mem(uint16_t addr, uint8_t *buf, uint16_t len)
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

// Write specified number of bytes to external memory from a buffer
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