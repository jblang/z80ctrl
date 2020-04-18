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
 * @file sioemu.h Serial I/O emulation
 */

#ifndef SIOEMU_H
#define SIOEMU_H

#ifndef SIO_BASE
//#define SIO_BASE 0x80 /*0x10*/
#endif

//#define SIO0_STATUS SIO_BASE+2
//#define SIO0_DATA SIO_BASE+3
//#define SIO1_STATUS SIO_BASE
//#define SIO1_DATA SIO_BASE+1


#define SIO_UART0 0
#define SIO_UART1 1
#define SIO_FILE 0xfe
#define SIO_UNATTACHED 0xff

#define SIO_OUTPUT 0
#define SIO_INPUT 1

typedef uint8_t (*port_rd)(uint8_t);
typedef void (*port_wr)(uint8_t, uint8_t);

typedef struct tag_device{
	char *name;
	char *desc;
	uint8_t id;
	uint8_t addr;
	uint8_t uart;  //used to map the given periphetal to any atmega uart. 0=UART0 on z80ctlr, 1=UART1 on z80ctrl
	port_rd p_read;   
    port_wr p_write;   
}DEVICE;

//Device Type IDs
enum{
	SIO0_STATUS,
	SIO0_DATA,
	SIO1_STATUS,
	SIO1_DATA
};

void sio_attach(uint8_t port, uint8_t dir, uint8_t mode, char *filename) ;
uint8_t sio_read(uint8_t port);
void sio_write(uint8_t port, uint8_t data);
uint8_t sio_status(uint8_t port);

extern DEVICE *write_dev_tbl[256];
extern DEVICE *read_dev_tbl[256];

#endif
