#ifndef IOX_H
#define IOX_H

#include <stdint.h>

// Register addresses in bank 0
#define IODIRA0 0x00
#define IODIRB0 0x01
#define IPOLA0 0x02
#define IPOLB0 0x03
#define GPINTENA0 0x04
#define GPINTENB0 0x05
#define DEFVALA0 0x06
#define DEFVALB0 0x07
#define INTCONA0 0x08
#define INTCONB0 0x09
#define IOCON0 0x0A
#define GPPUA0 0x0C
#define GPPUB0 0x0D
#define INTFA0 0x0E
#define INTFB0 0x0F
#define INTCAPA0 0x10
#define INTCAPB0 0x11
#define GPIOA0 0x12
#define GPIOB0 0x13
#define OLATA0 0x14
#define OLATB0 0x15

// Register addresses in bank 1
#define IODIRA1 0x00
#define IPOLA1 0x01
#define GPINTENA1 0x02
#define DEFVALA1 0x03
#define INTCONA1 0x04
#define IOCON1 0x05
#define GPPUA1 0x06
#define INTFA1 0x07
#define INTCAPA1 0x08
#define GPIOA1 0x09
#define OLATA1 0x0A
#define IODIRB1 0x10
#define IPOLB1 0x11
#define GPINTENB1 0x12
#define DEFVALB1 0x13
#define INTCONB1 0x14
#define GPPUB1 0x16
#define INTFB1 0x17
#define INTCAPB1 0x18
#define GPIOB1 0x19
#define OLATB1 0x1A

// Bits in IOCON register
#define INTCC 0
#define INTPOL 1
#define ODR 2
#define SEQOP 5
#define MIRROR 6
#define BANK 7

#define SPI_ADDR 0x40
#define WRITE 0
#define READ 1

void iox_init(void);
void iox_begin(uint8_t mode, uint8_t addr);
void iox_end(void);
uint8_t iox_read(uint8_t addr);
void iox_write(uint8_t addr, uint8_t data);

#endif
