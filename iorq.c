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
#include "iox.h"
#include "rtc.h"
#include "diskemu.h"
#include "sioemu.h"
#include "filedma.h"
#include "bdosemu.h"
#ifdef MSX_KEY_BASE
#include "msxkey.h"
#endif

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

/**
 * Function pointer of to DMA transfer function to be run after IORQ is acknowledged
 */
void (*dma_function)(void) = NULL;

/**
 * IO registers
 */

#ifdef IOX_BASE
#define IOX_DEVPORT IOX_BASE
#define IOX_REGPORT IOX_BASE+1
#define IOX_VALPORT IOX_BASE+2
#define IOX_RTC 0
#define IOX_GPIO_MIN 1
#define IOX_GPIO_MAX 7

static uint8_t iox_dev = 0;
static uint8_t iox_reg = 0;
#endif

/**
 * Handle Z80 IO request
 */
void iorq_dispatch(uint8_t logged)
{
    //cli();
    switch (GET_ADDRLO) {
#ifdef IOX_BASE
        case IOX_DEVPORT:
            if (!GET_WR) {
                iox_dev = GET_DATA;
            }
            break;
        case IOX_REGPORT:
            if (!GET_WR) {
                iox_reg = GET_DATA;
            }
            break;
        case IOX_VALPORT:
            if (!GET_WR) {
                if (iox_dev == IOX_RTC) {
#ifdef DS1306_RTC
                    rtc_write1(iox_reg, GET_DATA);
#endif
                } else if (iox_dev >= IOX_GPIO_MIN && iox_dev <= IOX_GPIO_MAX) {
                    iox_write(iox_dev, iox_reg, GET_DATA);
                }
            } else if (!GET_RD) {
                if (iox_dev == IOX_RTC) {
#ifdef DS1306_RTC
                    SET_DATA(rtc_read1(iox_reg));
#endif
                } else if (iox_dev >= IOX_GPIO_MIN && iox_dev <= IOX_GPIO_MAX) {
                    SET_DATA(iox_read(iox_dev, iox_reg));
                }
                DATA_OUTPUT;
            }
            break;
#endif
        case SIO0_STATUS:
            if (!GET_RD) {
                SET_DATA(sio_status(0));
                DATA_OUTPUT;
            }
            break;
        case SIO1_STATUS:
            if (!GET_RD) {
                SET_DATA(sio_status(1));
                DATA_OUTPUT;
            }
            break;
        case SIO0_DATA:
            if (!GET_RD) {
                SET_DATA(sio_read(0));
                DATA_OUTPUT;
            } else if (!GET_WR) {
                sio_write(0, GET_DATA);
            }
            break;
        case SIO1_DATA:
            if (!GET_RD) {
                SET_DATA(sio_read(1));
                DATA_OUTPUT;
            } else if (!GET_WR) {
                sio_write(1, GET_DATA);
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
        case FILE_DMA:
            if (!GET_RD) { 
                SET_DATA(file_dma_reset());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                file_dma_command(GET_DATA);
            }
            break;
        case BDOS_DMA:
            if (!GET_RD) { 
                SET_DATA(bdos_dma_reset());
                DATA_OUTPUT;
            } else if (!GET_WR) {
                bdos_dma_command(GET_DATA);
            }
            break;
#ifdef MSX_KEY_BASE
        case MSX_KEY_COL:
            if (!GET_RD) {
                SET_DATA(msx_scanrow());
                DATA_OUTPUT;
            }
            break;
        case MSX_KEY_ROW:
            if (!GET_WR) {
                msx_setrow(GET_DATA);
            } 
            break;
#endif
        default:
            if (!GET_RD) {
                SET_DATA(0xFF);
            }
    }
    if (logged) {
        iorq_stat = bus_status_fast();
    }
    BUSRQ_LO;
    while (!GET_IORQ)
        CLK_TOGGLE;
    if (dma_function) {
        if (bus_master())
            dma_function();
        dma_function = NULL;
        bus_slave();
    }
    DATA_INPUT;
    if (!GET_BUSACK)
        bus_slave();
    BUSRQ_HI;
    //sei();
}
