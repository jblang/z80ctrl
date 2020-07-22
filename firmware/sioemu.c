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
 * @file sioemu.c Serial I/O emulation
 */

#include <stdint.h>
#include <avr/pgmspace.h>

#include "ff.h"
#include "sioemu.h"
#include "uart.h"
#include "ffwrap.h"

/**
 * Physical to virtual UART mapping.
 */

static FIL sio_writefile[2];
static FIL sio_readfile[2];

static uint8_t sio_writemode[2] = {SIO_UART0, SIO_UART1};
static uint8_t sio_readmode[2] = {SIO_UART0, SIO_UART1};

#define SIO_EOF 0x1A

void sio_unattach(uint8_t port, uint8_t dir)
{
    FRESULT fr;
    if (port > 1) {
        printf_P(PSTR("error: valid port numbers are 0-1\n"));
        return;
    }

    uint8_t *sio_mode = sio_readmode;
    FIL *sio_file = sio_readfile;

    if (dir == SIO_OUTPUT) {
        sio_mode = sio_writemode;
        sio_file = sio_writefile;
    }
    
    if (sio_mode[port] == SIO_FILE)
        file_close(&sio_file[port]);
    sio_mode[port] = SIO_UNATTACHED;        
}

/**
 * Attach a file to serial port
 */
void sio_attach(uint8_t port, uint8_t dir, uint8_t mode, char *filename) 
{
    if (port > 1) {
        printf_P(PSTR("error: valid port numbers are 0-1\n"));
        return;
    }
    uint8_t *sio_mode = sio_readmode;
    FIL *sio_file = sio_readfile;

    if (dir == SIO_OUTPUT) {
        sio_mode = sio_writemode;
        sio_file = sio_writefile;
    }

    if (sio_mode[port] == SIO_FILE)
        sio_unattach(port, dir);

    FRESULT fr;
    sio_mode[port] = mode;
    if (mode == SIO_FILE) {
        if ((fr = file_open(&sio_file[port], NULL, filename, (dir == SIO_INPUT ? FA_READ : FA_WRITE) | FA_OPEN_ALWAYS)) != FR_OK)
            sio_mode[port] = SIO_UNATTACHED;
    }
}

/**
 * Read from serial port
 */
uint8_t sio_read(uint8_t port)
{
    UINT br;
    FRESULT fr;
    uint8_t data;
    if (port > 1) {
        printf_P(PSTR("error: valid port numbers are 0-1\n"));
        return 0;
    }
    if (sio_readmode[port] == SIO_FILE) {
        if ((fr = file_read(&sio_readfile[port], &data, 1, &br)) != FR_OK)
            return 0;
        if (br == 0)
            return SIO_EOF;
        return data;
    } else if (sio_readmode[port] != SIO_UNATTACHED) {
            return uart_getc(sio_readmode[port]);
    }
    return 0;
}
uint8_t sio0_read()
{
    return sio_read(0);
}
uint8_t sio1_read()
{
    return sio_read(1);
}

/**
 * Write to serial port
 */
void sio_write(uint8_t port, uint8_t data)
{
    UINT bw;
    FRESULT fr;
    if (port > 1) {
        printf_P(PSTR("error: valid port numbers are 0-1\n"));
        return;
    }
    if (sio_writemode[port] == SIO_FILE) {
        if ((fr = file_write(&sio_writefile[port], &data, 1, &bw)) != FR_OK)
            return;
    } else if (sio_writemode[port] != SIO_UNATTACHED) {
        uart_putc(sio_writemode[port], data);
    }
}
void sio0_write(uint8_t data)
{
    sio_write(0, data);
}
void sio1_write(uint8_t data)
{
    sio_write(1, data);
}

/**
 * Get serial port status
 */
uint8_t sio_status(uint8_t port)
{
    UINT bw;
    FRESULT fr;
    uint8_t status = 0;
    if (port > 1) {
        printf_P(PSTR("error: valid port numbers are 0-1\n"));
        return 0;
    }
    if (sio_writemode[port] == SIO_FILE)
        status |= (1 << 1);
    else if (sio_writemode[port] != SIO_UNATTACHED)
        status |= ((uart_testtx(sio_writemode[port]) == 0)) << 1;
    
    if (sio_readmode[port] == SIO_FILE)
        status |= 1;
    else if (sio_readmode[port] != SIO_UNATTACHED)
        status |= (uart_testrx(sio_readmode[port]) > 0);
    return status;
}
uint8_t sio0_status()
{
    return sio_status(0);
}
uint8_t sio1_status()
{
    return sio_status(1);
}