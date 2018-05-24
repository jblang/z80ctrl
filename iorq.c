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
 * @file iorq.c I/O request handler
 */

#include "iorq.h"
#include "uart.h"
#include "bus.h"
#include "diskemu.h"

void (*dma_function)(void) = NULL;

/**
 * IO registers
 */
#define SIO0_STATUS 0x10
#define SIO0_DATA 0x11
#define SIO1_STATUS 0x12
#define SIO1_DATA 0x13
#define SIOA_CONTROL 0x80
#define SIOA_DATA 0x81
#define SIOB_CONTROL 0x82
#define SIOB_DATA 0x83
#define HDISK_IO 0xFD
#define SIMH_DEV 0xFE
#define SENSE_SW 0xFF

/**
 * Physical to virtual UART mapping.
 */
uint8_t z80_uart[] = {0 , 1};

/**
 * Utility macros to generate SIO status register values
 */
#define ALTAIR_SIO_STATUS(u) ((((uart_testtx(z80_uart[(u)]) == 0) << 1) & 0x2) | ((uart_testrx(z80_uart[(u)]) > 0) & 0x1))
#define ZILOG_SIO_STATUS(u) ((1 << 3) | (1  << 5) | (((uart_testtx(z80_uart[(u)]) == 0) << 2) & 0x4) | ((uart_testrx(z80_uart[(u)]) > 0) & 0x1))

/**
 * Handle Z80 IO request
 */
void iorq_dispatch(void)
{
    switch (GET_ADDRLO) {
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA(ALTAIR_SIO_STATUS(0));
                DATA_OUTPUT;
            }
            break;
        case SIO1_STATUS:
            if (!GET_RD) {
                SET_DATA(ALTAIR_SIO_STATUS(1));
                DATA_OUTPUT;
            }
            break;
        case SIOA_CONTROL:
            if (!GET_RD) {
                SET_DATA(ZILOG_SIO_STATUS(0));
                DATA_OUTPUT;
            }
            break;
        case SIOB_CONTROL:
            if (!GET_RD) {
                SET_DATA(ZILOG_SIO_STATUS(1));
                DATA_OUTPUT;
            }
            break;
        case SIO0_DATA:
        case SIOA_DATA:
            if (!GET_RD) {
                SET_DATA(uart_getc(z80_uart[0]));
                DATA_OUTPUT;
            } else if (!GET_WR) {
                uart_putc(z80_uart[0], GET_DATA);
            }
            break;
        case SIO1_DATA:
        case SIOB_DATA:
            if (!GET_RD) {
                SET_DATA(uart_getc(z80_uart[1]));
                DATA_OUTPUT;
            } else if (!GET_WR) {
                uart_putc(z80_uart[1], GET_DATA);
            }
            break;
        case DRIVE_STATUS:
            if (!GET_RD) {
                SET_DATA(drive_status());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_select(GET_DATA);
            }
            break;
        case DRIVE_CONTROL:
            if (!GET_RD) {
                SET_DATA(drive_sector());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_control(GET_DATA);
            }
            break;
        case DRIVE_DATA:
            if (!GET_RD) {
                SET_DATA(drive_read());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_write(GET_DATA);
            }
            break;
        case HDISK_IO:
            if (!GET_RD) { 
                SET_DATA(hdsk_in());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                hdsk_out(GET_DATA);
            }
            break;
        default:
            if (!GET_RD) {
                SET_DATA(0xFF);
            }
    }
    if (dma_function != NULL) {
        dma_function();
        dma_function = NULL;
    } else {
        if (!GET_RD)
            BUSRQ_LO;
        IOACK_LO;
        while (!GET_IORQ) {
            CLK_TOGGLE;
        }
        DATA_INPUT;
        IOACK_HI;
        BUSRQ_HI;
    }
}
