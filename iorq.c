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
#include "sioemu.h"

#include <avr/interrupt.h>

/**
 * Function pointer of to DMA transfer function to be run after IORQ is acknowledged
 */
void (*dma_function)(void) = NULL;

/**
 * IO registers
 */
#define SIMH_DEV 0xFE
#define SENSE_SW 0xFF

/**
 * Handle Z80 IO request
 */
void iorq_dispatch(void)
{
    cli();
    switch (GET_ADDRLO) {
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA(ACIA_STATUS(0));
                DATA_OUTPUT;
            }
            break;
        case SIO1_STATUS:
            if (!GET_RD) {
                SET_DATA(ACIA_STATUS(1));
                DATA_OUTPUT;
            }
            break;
        case SIOA_CONTROL:
            if (!GET_RD) {
                SET_DATA(ZSIO_STATUS(0));
                DATA_OUTPUT;
            }
            break;
        case SIOB_CONTROL:
            if (!GET_RD) {
                SET_DATA(ZSIO_STATUS(1));
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
        case DRIVE_DMA:
            if (!GET_RD) { 
                SET_DATA(drive_dma_result());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                drive_dma_command(GET_DATA);
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
        uint8_t ddr = DATA_DDR;
        if (ddr)
            BUSRQ_LO;
        IOACK_LO;
        while (!GET_IORQ)
            CLK_TOGGLE;
        DATA_INPUT;
        IOACK_HI;
        if (ddr)
            BUSRQ_HI;
    }
    sei();
}
