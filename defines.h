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
#define IOX_CS 4
#define SD_CS 3

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

// CPU control lines on local port
#define CTRL_DDR DDRD
#define CTRL_PORT PORTD
#define CTRL_PIN PIND
#define IORQ 2
#define IORQ_vect INT1_vect
#define MREQ 3
#define WR 4
#define RD 5
#define CLK 6
#define IOACK 7

#define CTRL2_DDR DDRB
#define CTRL2_PORT PORTB
#define CTRL2_PIN PINB
#define M1 1
//#define RFSH 2
#define HALT 2

// CPU control lines on i/o expander
#define CTRLX_IODIR IODIRB0
#define CTRLX_GPPU GPPUB0
#define CTRLX_GPIO GPIOB0
#define INT 0
#define NMI 1
#define RESET 2
#define BUSRQ 3
#define BUSACK 4
#define BANKADDR 5
#define BANKMASK ((1<<BANKADDR) | (1<<(BANKADDR+1)) | (1<<(BANKADDR+2)))

#define DEBUG

#endif
