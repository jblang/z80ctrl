/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
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
 * @file io.c I/O request handler
 */

#ifndef HARDWARE_IO_H
#define HARDWARE_IO_H

#include <stdint.h>

#include "bus.h"

typedef enum {
    DEV_UNASSIGNED,

    EMU_ACIA0_STATUS,
    EMU_ACIA0_DATA,
    EMU_ACIA1_STATUS,
    EMU_ACIA1_DATA,

    EMU_88DSK_STATUS,
    EMU_88DSK_CONTROL,
    EMU_88DSK_DATA,

#ifdef MSX_KEY_BASE
    EMU_MSXKEY_COL,
    EMU_MSXKEY_ROW,
#endif

    Z80CTRL_DEV,
    Z80CTRL_REG,
    Z80CTRL_LO,
    Z80CTRL_HI,
    Z80CTRL_FATFS_DMA,
    Z80CTRL_BDOS_EMU,

    EXT_UNKNOWN,

#ifdef BANK_BASE
    EXT_WBW_RAM,
#endif

#ifdef TMS_BASE
    EXT_TMS_RAM,
    EXT_TMS_REG,
#endif

#ifdef SN76489_PORT
    EXT_SN76489,
#endif 

    DEV_INVALID
} device_type;

typedef enum {
    IORQ_READ = 1,
    IORQ_WRITE,
    IORQ_RW
} device_mode;

void iorq_init();
void iorq_list();
uint8_t iorq_dispatch();
uint8_t iorq_deviceid(char *name);
uint8_t iorq_assign(uint8_t port, device_mode mode, device_type device);

uint8_t io_out(uint8_t addr, uint8_t value);
uint8_t io_in(uint8_t addr);

typedef enum {
    DMA_MAILBOX_UNSET = 0,
    DMA_MAILBOX_HALFSET,
    DMA_MAILBOX_SET
} dma_status_t;

extern void (*dma_function)(void);

 #endif