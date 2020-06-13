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
#include "util.h"
#include "diskemu.h"
#include "sioemu.h"
#include "filedma.h"
#include "bdosemu.h"
#ifdef MSX_KEY_BASE
#include "msxkey.h"
#endif

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

/**
 * Function pointer of to DMA transfer function to be run after IORQ is acknowledged
 */
void (*dma_function)(void) = NULL;

static uint8_t write_port[256];
static uint8_t read_port[256];

const char device_name[] PROGMEM =
    "none\0"

    "uart0s\0"
    "uart0d\0"
    "uart1s\0"
    "uart1d\0"

    "dskstat\0"
    "dskctrl\0"
    "dskdata\0"

    "msxcol\0"
    "msxrow\0"

    "spidev\0"
    "spireg\0"
    "spidata\0"

    "fatfs\0"
    "bdos\0"

    "unknown\0"

    "wbwpage\0"

    "tmsram\0"
    "tmsreg\0"

    "sn76489\0";

const char device_description[] PROGMEM = 
    "Unassigned\0"

    "UART 0 status\0"
    "UART 0 data\0"
    "UART 1 status\0"
    "UART 1 data\0"

    "88-DSK status\0"
    "88-DSK control\0"
    "88-DSK data\0"

#ifdef MSX_KEY_BASE
    "MSX keyboard column\0"
    "MSX keyboard row\0"
#endif

#ifdef IOX_BASE
    "SPI device select\0"
    "SPI register select\0"
    "SPI data\0"
#endif

    "FatFS DMA control\0"
    "BDOS DMA control\0"

    "External device\0"

    "512K ROM/RAM page\0"

    "TMS9918A RAM\0"
    "TMS9918A register\0"

    "SN76489\0";

void * const device_read[] PROGMEM = {
    NULL,               // DEV_UNASSIGNED

    &sio0_status,       // EMU_ACIA_STATUS
    &sio0_read,         // EMU_ACIA_DATA
    &sio1_status,       // EMU_ACIA_STATUS
    &sio1_read,         // EMU_ACIA_DATA

    &drive_status,      // EMU_88DSK_STATUS
    &drive_sector,      // EMU_88DSK_CONTROL
    &drive_read,        // EMU_88DSK_DATA

#ifdef MSX_KEY_BASE
    &msx_scanrow,       // EMU_MSX_KEY_COL
    NULL,               // EMU_MSX_KEY_ROW
#endif

#ifdef IOX_BASE
    &iox_getdev,        // Z80CTRL_IOX_DEV
    &iox_getreg,        // Z80CTRL_IOX_REG
    &iox_readval,       // Z80CTRL_IOX_VAL
#endif

    &file_dma_reset,    // Z80CTRL_FATFS_DMA
    &bdos_dma_reset,    // Z80CTRL_BDOS_EMU

    NULL                // EXT_UNKNOWN
};

void * const device_write[] PROGMEM = {
    NULL,               // DEV_UNASSIGNED

    NULL,               // EMU_ACIA_STATUS
    &sio0_write,        // EMU_ACIA_DATA 
    NULL,               // EMU_ACIA_STATUS
    &sio1_write,        // EMU_ACIA_DATA 

    &drive_select,      // EMU_88DSK_STATUS 
    &drive_control,     // EMU_88DSK_CONTROL 
    &drive_write,       // EMU_88DSK_DATA 

#ifdef MSX_KEY_BASE
    NULL,               // EMU_MSX_KEY_COL
    &msx_setrow,        // EMU_MSX_KEY_ROW 
#endif

#ifdef IOX_BASE
    &iox_setdev,        // Z80CTRL_IOX_DEV 
    &iox_setreg,        // Z80CTRL_IOX_REG 
    &iox_writeval,      // Z80CTRL_IOX_VAL 
#endif

    &file_dma_command,  // Z80CTRL_FATFS_DMA 
    &bdos_dma_command,  // Z80CTRL_BDOS_EMU 

    NULL                // EXT_UNKNOWN
};

void iorq_list()
{
    uint16_t start;
    for (uint16_t i = 0; i <= 0xff; i++) {
        if (i == 0 || write_port[start] != write_port[i] || read_port[start] != read_port[i])
            start = i;
        if (write_port[i] != DEV_UNASSIGNED || read_port[i] != DEV_UNASSIGNED) {
            if (start == 255 || write_port[i+1] != write_port[i] || read_port[i+1] != read_port[i]) {
                if (start != i)
                    printf_P(PSTR("%02X-"), start);
                else
                    printf_P(PSTR("   "));
                if (read_port[i] == write_port[i]) {
                    printf_P(PSTR("%02X RW %-8S %S\n"), i,
                            strlookup(device_name, read_port[i]),
                            strlookup(device_description, read_port[i]));
                } else {
                    if (read_port[i] != DEV_UNASSIGNED)
                        printf_P(PSTR("%02X R  %-8S %S\n"), i, 
                            strlookup(device_name, read_port[i]),
                            strlookup(device_description, read_port[i]));
                    if (write_port[i] != DEV_UNASSIGNED)
                        printf_P(PSTR("%02X  W %-8S %S\n"), i, 
                            strlookup(device_name, read_port[i]),
                            strlookup(device_description, read_port[i]));
                }
            }
        }
    }
}

uint8_t iorq_deviceid(char *name)
{
    for (uint8_t i = 0; i < DEV_INVALID; i++) {
        if (strcmp_P(name, strlookup(device_name, i)) == 0)
            return i;
    }
    return DEV_INVALID;
}

uint8_t iorq_assign(uint8_t port, device_mode mode, device_type device)
{
    if (mode & IORQ_READ) {
        // Don't assign of internal device to port owned by external device
        if (read_port[port] >= EXT_UNKNOWN && device < EXT_UNKNOWN)
            return read_port[port];
        read_port[port] = device;
    }
    if (mode & IORQ_WRITE) {
        write_port[port] = device;
    }
    return device;
}

void iorq_init()
{
    if (GET_BUSACK)
        return;

    // Search for external devices
    clk_run();
    DATA_INPUT;
    SET_DATA(0xFF);
    IORQ_LO;
    for (uint16_t i = 0; i <= 0xff; i++) {
        write_port[i] = DEV_UNASSIGNED;
        read_port[i] = DEV_UNASSIGNED;
        SET_ADDRLO(i);
        RD_LO;
        _delay_us(10);
        if (GET_DATA != 0xFF)
            read_port[i] = EXT_UNKNOWN;
        RD_HI;
        _delay_us(10);
    }
    IORQ_HI;
    clk_stop();

    // Assign default ports for SPI peripherals
#ifdef IOX_BASE
    if (read_port[IOX_BASE] == DEV_UNASSIGNED
            && read_port[IOX_BASE+1] == DEV_UNASSIGNED
            && read_port[IOX_BASE+2] == DEV_UNASSIGNED) {
        read_port[IOX_BASE] = Z80CTRL_SPI_DEV;
        write_port[IOX_BASE] = Z80CTRL_SPI_DEV;
        read_port[IOX_BASE+1] = Z80CTRL_SPI_REG;
        write_port[IOX_BASE+1] = Z80CTRL_SPI_REG;
        read_port[IOX_BASE+2] = Z80CTRL_SPI_DATA;
        write_port[IOX_BASE+2] = Z80CTRL_SPI_DATA;
    }
#endif

    // Assign default ports for emulated 88-SIO
    if (read_port[SIO0_DATA] == DEV_UNASSIGNED
            && read_port[SIO0_STATUS] == DEV_UNASSIGNED) {
        read_port[SIO0_DATA] = EMU_ACIA0_DATA;
        write_port[SIO0_DATA] = EMU_ACIA0_DATA;
        read_port[SIO0_STATUS] = EMU_ACIA0_STATUS;
        write_port[SIO0_STATUS] = EMU_ACIA0_STATUS;
    }
    if (read_port[SIO1_DATA] == DEV_UNASSIGNED
            && read_port[SIO1_STATUS] == DEV_UNASSIGNED) {
        read_port[SIO1_DATA] = EMU_ACIA1_DATA;
        write_port[SIO1_DATA] = EMU_ACIA1_DATA;
        read_port[SIO1_STATUS] = EMU_ACIA1_STATUS;
        write_port[SIO1_STATUS] = EMU_ACIA1_STATUS;
    }

    // Assign default ports for emulated 88-DSK
    if (read_port[DRIVE_STATUS] == DEV_UNASSIGNED
            && read_port[DRIVE_CONTROL] == DEV_UNASSIGNED
            && read_port[DRIVE_DATA] == DEV_UNASSIGNED) {
        read_port[DRIVE_STATUS] = EMU_88DSK_STATUS;
        write_port[DRIVE_STATUS] = EMU_88DSK_STATUS;
        read_port[DRIVE_CONTROL] = EMU_88DSK_CONTROL;
        write_port[DRIVE_CONTROL] = EMU_88DSK_CONTROL;
        read_port[DRIVE_DATA] = EMU_88DSK_DATA;
        write_port[DRIVE_DATA] = EMU_88DSK_DATA;
    }

    // Assign default DMA control ports
    if (read_port[FILE_DMA] == DEV_UNASSIGNED) {
        read_port[FILE_DMA] = Z80CTRL_FATFS_DMA;
        write_port[FILE_DMA] = Z80CTRL_FATFS_DMA;
    }
    if (read_port[BDOS_DMA] == DEV_UNASSIGNED) {
        read_port[BDOS_DMA] = Z80CTRL_BDOS_EMU;
        write_port[BDOS_DMA] = Z80CTRL_BDOS_EMU;
    }

    // Assign default MSX keyboard ports
#ifdef MSX_KEY_BASE
    if (read_port[MSX_KEY_COL] == DEV_UNASSIGNED
            && read_port[MSX_KEY_ROW] == DEV_UNASSIGNED) {
        read_port[MSX_KEY_COL] = EMU_MSXKEY_COL;
        write_port[MSX_KEY_COL] = EMU_MSXKEY_COL;
        read_port[MSX_KEY_ROW] = EMU_MSXKEY_ROW;
        write_port[MSX_KEY_ROW] = EMU_MSXKEY_ROW;
    }
#endif

}

/**
 * Handle Z80 IO request
 */
void iorq_dispatch(uint8_t logged)
{
    if (!GET_RD) {
        uint8_t devid = read_port[GET_ADDRLO];
        if (devid < EXT_UNKNOWN) {
            uint8_t (*read_func)() = pgm_read_ptr(&device_read[devid]);
            if (read_func != NULL) {
                SET_DATA(read_func());
                DATA_OUTPUT;
            } else {
                SET_DATA(0xFF);
            }
        }
    } else if (!GET_WR) {
        uint8_t devid = write_port[GET_ADDRLO];
        void (*write_func)(uint8_t data) = pgm_read_ptr(&device_write[devid]);
        if (write_func != NULL) {
            write_func(GET_DATA);
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
}
