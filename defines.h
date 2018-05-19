#ifndef DEFINES_H
#define DEFINES_H

#include <avr/io.h>

#include "iox.h"

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#ifndef BAUD
#define BAUD  115200
#endif

// SPI pins
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SCK 7
#define MISO 6
#define MOSI 5
#define CSADDR 3
#define CSADDRMASK ((1 << CSADDR) | (1 << CSADDR+1))

#ifdef CORRECT_SPIADDR
#define IOX_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x0 << CSADDR)
#define SD_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x1 << CSADDR)
#define AUX1_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x2 << CSADDR)
#define AUX2_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x3 << CSADDR)
#else
#define IOX_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x0 << CSADDR)
#define SD_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x2 << CSADDR)
#define AUX1_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x1 << CSADDR)
#define AUX2_SEL SPI_PORT = SPI_PORT & ~CSADDRMASK | (0x3 << CSADDR)
#endif

// Data bus on local port
#define DATA_DDR DDRC
#define DATA_PORT PORTC
#define DATA_PIN PINC

// Address bus LSB on local port
#define ADDRLO_DDR DDRA
#define ADDRLO_PORT PORTA
#define ADDRLO_PIN PINA

// Address bus MSB on i/o expander
#define ADDRHI_IODIR IODIRA0
#define ADDRHI_GPPU GPPUA0
#define ADDRHI_GPIO GPIOA0

// CPU control lines on local ports
#define IOACK_DDR DDRB
#define IOACK_PORT PORTB
#define IOACK_PIN PINB
#define IOACK 0

#define IORQ_DDR DDRB
#define IORQ_PORT PORTB
#define IORQ_PIN PINB
#define IORQ 1

#define M1_DDR DDRB
#define M1_PORT PORTB
#define M1_PIN PINB
#define M1 2

#define RD_DDR DDRD
#define RD_PORT PORTD
#define RD_PIN PIND
#define RD 4

#define WR_DDR DDRD
#define WR_PORT PORTD
#define WR_PIN PIND
#define WR 5

#define CLK_DDR DDRD
#define CLK_PORT PORTD
#define CLK_PIN PIND
#define CLK 6

#define HALT_DDR DDRD
#define HALT_PORT PORTD
#define HALT_PIN PIND
#define HALT 7

// CPU control lines on i/o expander
#define CTRLX_IODIR IODIRB0
#define CTRLX_GPPU GPPUB0
#define CTRLX_GPIO GPIOB0
//#define SDCD 0
#define RFSH 1
#define RESET 2
#define INTERRUPT 3
#define BUSACK 4
#define MREQ 5
#define BUSRQ 6
#define NMI 7
//#define BANKADDR 5
//#define BANKMASK ((1<<BANKADDR) | (1<<(BANKADDR+1)) | (1<<(BANKADDR+2)))

// Enable SD card support in fatfs
#define DRV_MMC 0

// Number of emulated disk drives
#define NUMDRIVES 26

#endif
