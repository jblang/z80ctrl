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

#include <stdio.h>
#include <util/delay.h>

#include "bdos.h"
#include "disk.h"
#include "filedma.h"
#ifdef MSX_KEY_BASE
#include "msxkey.h"
#endif
#include "sio.h"

#include "harvard.h"
#include "strutil.h"

#include "io.h"
#include "mcp23s17.h"
#ifdef USE_RTC
#include "rtc.h"
#endif
#include "avr_timer.h"
#ifdef TMS_BASE
#include "tms9918.h"
#endif
#include "avr_uart.h"

/**
 * Function pointer of to DMA transfer function to be run after IORQ is acknowledged
 */
void (*dma_function)(void) = NULL;

/**
 * Currently assigned ports
 */
static uint8_t write_port[256];
static uint8_t read_port[256];

/**
 * Device name for assign command
 */
const char device_name[] PROGMEM = 
    "none\0"

    "uart0s\0"
    "uart0d\0"
    "uart1s\0"
    "uart1d\0"

    "dskstat\0"
    "dskctrl\0"
    "dskdata\0"

#ifdef MSX_KEY_BASE
    "msxcol\0"
    "msxrow\0"
#endif

    "z8cdev\0"
    "z8creg\0"
    "z8cdlo\0"
    "z8cdhi\0"

    "fatfs\0"
    "bdos\0"

    "extern\0"

#ifdef BANK_BASE
    "wbwbank\0"
#endif

#ifdef TMS_BASE
    "tmsram\0"
    "tmsreg\0"
#endif

#ifdef SN76489_PORT
    "sn76489\0"
#endif

    "invalid\0";

/**
 * Device descriptions shown during listing
 */
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

    "z80ctrl device select\0"
    "z80ctrl register/command select\0"
    "z80ctrl data (lo)\0"
    "z80ctrl data (hi)\0"

    "FatFS DMA control\0"
    "BDOS DMA control\0"

    "External device\0"

#ifdef BANK_BASE
    "512K ROM/RAM bank\0"
#endif

#ifdef TMS_BASE
    "TMS9918A RAM\0"
    "TMS9918A register\0"
#endif

#ifdef SN76489_PORT
    "SN76489\0"
#endif

    "Invalid device\0";

void z8c_setdev(uint8_t dev);
uint8_t z8c_getdev();
void z8c_setreg(uint8_t reg);
uint8_t z8c_getreg();
void z8c_setlo(uint8_t data);
uint8_t z8c_getlo();
void z8c_sethi(uint8_t data);
uint8_t z8c_gethi();

/**
 * Device read functions
 */
void* const device_read[] PROGMEM = {
    NULL, // DEV_UNASSIGNED

    &sio0_status,   // EMU_ACIA_STATUS
    &sio0_read,     // EMU_ACIA_DATA
    &sio1_status,   // EMU_ACIA_STATUS
    &sio1_read,     // EMU_ACIA_DATA

    &drive_status,  // EMU_88DSK_STATUS
    &drive_sector,  // EMU_88DSK_CONTROL
    &drive_read,    // EMU_88DSK_DATA

#ifdef MSX_KEY_BASE
    &msx_scanrow,   // EMU_MSX_KEY_COL
    NULL,           // EMU_MSX_KEY_ROW
#endif

    &z8c_getdev,    // Z80CTRL_DEV
    &z8c_getreg,    // Z80CTRL_REG
    &z8c_setlo,     // Z80CTRL_LO
    &z8c_sethi,     // Z80CTRL_HI
    &file_dma_reset, // Z80CTRL_FATFS_DMA
    &bdos_dma_reset, // Z80CTRL_BDOS_EMU

    NULL,           // EXT_UNKNOWN

#ifdef BANK_BASE
    NULL,           // EXT_WBW_RAM
#endif

#ifdef TMS_BASE
    NULL,           // EXT_TMS_RAM
    &tms_save_status, // EXT_TMS_REG
#endif

#ifdef SN76489_PORT
    NULL,           // EXT_SN76489
#endif

    NULL // DEV_INVALID
};

/**
 * Device write functions
 */
void* const device_write[] PROGMEM = {
    NULL,           // DEV_UNASSIGNED

    NULL,           // EMU_ACIA_STATUS
    &sio0_write,    // EMU_ACIA_DATA
    NULL,           // EMU_ACIA_STATUS
    &sio1_write,    // EMU_ACIA_DATA

    &drive_select,  // EMU_88DSK_STATUS
    &drive_control, // EMU_88DSK_CONTROL
    &drive_write,   // EMU_88DSK_DATA

#ifdef MSX_KEY_BASE
    NULL,           // EMU_MSX_KEY_COL
    &msx_setrow,    // EMU_MSX_KEY_ROW
#endif

    &z8c_setdev,    // Z80CTRL_DEV
    &z8c_setreg,    // Z80CTRL_REG
    &z8c_getlo,     // Z80CTRL_LO
    &z8c_gethi,     // Z80CTRL_HI
    &file_dma_command, // Z80CTRL_FATFS_DMA
    &bdos_dma_command, // Z80CTRL_BDOS_EMU

    NULL,           // EXT_UNKNOWN

#ifdef BANK_BASE
    NULL,           // EXT_WBW_RAM
#endif

#ifdef TMS_BASE
    NULL,           // EXT_TMS_RAM
    &tms_save_reg,  // EXT_TMS_REG
#endif

#ifdef SN76489_PORT
    NULL,           // EXT_SN76489
#endif

    NULL            // DEV_INVALID
};

/**
 * Lists devices assigned to each port
 */
void iorq_list()
{
    uint16_t start;
    for (uint16_t i = 0; i <= 0xff; i++) {
        if (i == 0 || write_port[start] != write_port[i] || read_port[start] != read_port[i])
            start = i;
        if (write_port[i] != DEV_UNASSIGNED || read_port[i] != DEV_UNASSIGNED) {
            if (start == 255 || write_port[i + 1] != write_port[i] || read_port[i + 1] != read_port[i]) {
                if (start != i)
                    printf_P(PSTR("%02X-"), start);
                else
                    printf_P(PSTR("   "));
                if (read_port[i] == write_port[i]) {
                    printf_P(PSTR("%02X RW %-8S %S\n"), i,
                        skip_strings(device_name, read_port[i]),
                        skip_strings(device_description, read_port[i]));
                } else {
                    if (read_port[i] != DEV_UNASSIGNED)
                        printf_P(PSTR("%02X R  %-8S %S\n"), i,
                            skip_strings(device_name, read_port[i]),
                            skip_strings(device_description, read_port[i]));
                    if (write_port[i] != DEV_UNASSIGNED)
                        printf_P(PSTR("%02X  W %-8S %S\n"), i,
                            skip_strings(device_name, read_port[i]),
                            skip_strings(device_description, read_port[i]));
                }
            }
        }
    }
}

/**
 * Returns device ID given a device name
 */
uint8_t iorq_deviceid(char* name)
{
    for (uint8_t i = 0; i < DEV_INVALID; i++) {
        if (strcmp_P(name, skip_strings(device_name, i)) == 0)
            return i;
    }
    return DEV_INVALID;
}

/**
 * Assign a device to a port
 */
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

/**
 * Default ports for internal devices
 */
const uint8_t default_ports[] PROGMEM = {
    0x00, Z80CTRL_DEV,
    0x01, Z80CTRL_REG,
    0x02, Z80CTRL_LO,
    0x03, Z80CTRL_HI,

    0x10, EMU_ACIA0_STATUS,
    0x11, EMU_ACIA0_DATA,
    0x12, EMU_ACIA1_STATUS,
    0x13, EMU_ACIA1_DATA,
    0x80, EMU_ACIA0_STATUS,
    0x81, EMU_ACIA0_DATA,
    0x40, EMU_ACIA1_STATUS,
    0x41, EMU_ACIA1_DATA,

    0x08, EMU_88DSK_STATUS,
    0x09, EMU_88DSK_CONTROL,
    0x0A, EMU_88DSK_DATA,

    0x0B, Z80CTRL_FATFS_DMA,
    0x0C, Z80CTRL_BDOS_EMU,

#ifdef MSX_KEY_BASE
    0xA9, EMU_MSXKEY_COL,
    0xAA, EMU_MSXKEY_ROW
#endif
};

#define NUM_DEFAULTS (sizeof(default_ports) / sizeof(uint8_t))

/**
 * Initialize I/O map
 */
void iorq_init()
{
    if (GET_BUSACK)
        return;

    // Search for external devices
    clk_run();
    DATA_INPUT;
    SET_DATA(0xFF); // set pullups
    IORQ_LO;
    for (uint16_t i = 0; i <= 0xff; i++) {
        SET_ADDRLO(i);
        RD_LO;
        _delay_us(10);
        // if no device responds, pullups will force data bus to 0xff
        // so if the data bus is not 0xff, we know a device is there
        if (GET_DATA != 0xFF) {
            read_port[i] = EXT_UNKNOWN;
            write_port[i] = EXT_UNKNOWN;
        } else {
            read_port[i] = DEV_UNASSIGNED;
            write_port[i] = DEV_UNASSIGNED;
        }
        RD_HI;
        _delay_us(10);
    }
    IORQ_HI;
    clk_stop();

    // Assign default ports to devices as long as they are unused
    for (uint16_t i = 0; i < NUM_DEFAULTS; i += 2) {
        uint8_t port = pgm_read_byte(&default_ports[i]);
        uint8_t device = pgm_read_byte(&default_ports[i + 1]);
        if (read_port[port] == DEV_UNASSIGNED && write_port[port] == DEV_UNASSIGNED) {
            read_port[port] = device;
            write_port[port] = device;
        }
    }

#ifdef TMS_BASE
    uint8_t tms_port = tms_detect();
    if (tms_port != 1) {
        read_port[tms_port] = EXT_TMS_RAM;
        write_port[tms_port] = EXT_TMS_RAM;
        read_port[tms_port + 1] = EXT_TMS_REG;
        write_port[tms_port + 1] = EXT_TMS_REG;
    }
#endif
}

/**
 * Handle Z80 IO request
 */
uint8_t iorq_dispatch()
{
    if (!GET_RD) {
        // set pullups in case no device is present
        SET_DATA(0xFF);
        uint8_t devid = read_port[GET_ADDRLO];
        if (devid < EXT_UNKNOWN) {
            // output data from internal device
            uint8_t (*read_func)() = pgm_read_ptr(&device_read[devid]);
            if (read_func != NULL) {
                SET_DATA(read_func());
                DATA_OUTPUT;
            }
        } else {
            // capture data from external device
            void (*save_func)(uint8_t data) = pgm_read_ptr(&device_read[devid]);
            if (save_func != NULL)
                save_func(GET_DATA);
        }
    } else if (!GET_WR) {
        uint8_t devid = write_port[GET_ADDRLO];
        void (*write_func)(uint8_t data) = pgm_read_ptr(&device_write[devid]);
        if (write_func != NULL) {
            write_func(GET_DATA);
        }
    }
    uint8_t data = GET_DATA;
    if (dma_function) {
        if (bus_request())
            dma_function();
        else
            printf_P(PSTR("bus request timed out"));
        dma_function = NULL;
        bus_release();
    } else {
        BUSRQ_LO;
        while (GET_BUSACK)
            CLK_TOGGLE;
        DATA_INPUT;
        BUSRQ_HI;
    }
    return data;
}

/**
 * Output value to an IO register
 */
uint8_t io_out(uint8_t addr, uint8_t value)
{
    if (GET_BUSACK)
        return 0;
    MREQ_HI;
    RD_HI;
    _delay_us(1);
    SET_ADDRLO(addr);
    SET_DATA(value);
    DATA_OUTPUT;
    IORQ_LO;
    WR_LO;
    _delay_us(1);
    WR_HI;
    _delay_us(1);
    IORQ_HI;
    DATA_INPUT;
    return 1;
}

uint32_t base_addr = 0;

/**
 * Input value from an IO register
 */
uint8_t io_in(uint8_t addr)
{
    if (GET_BUSACK)
        return 0;
    MREQ_HI;
    WR_HI;
    _delay_us(1);
    SET_ADDRLO(addr);
    DATA_INPUT;
    IORQ_LO;
    RD_LO;
    _delay_us(1);
    uint8_t value = GET_DATA;
    RD_HI;
    IORQ_HI;
    return value;
}

#define Z8C_RTC 0x00
#define Z8C_GPIO_MIN 0x01
#define Z8C_GPIO_MAX 0x07
#define Z8C_BDOS_EMU 0x08

static uint8_t z8c_dev = 0;
static uint8_t z8c_reg = 0;

void z8c_setdev(uint8_t dev)
{
    z8c_dev = dev;
}

uint8_t z8c_getdev()
{
    return z8c_dev;
}

void z8c_setreg(uint8_t reg)
{
    z8c_reg = reg;
}

uint8_t z8c_getreg()
{
    return z8c_reg;
}

void z8c_setlo(uint8_t data)
{
    if (z8c_dev == Z8C_RTC) {
#ifdef USE_RTC
        rtc_write(z8c_reg, data);
#endif
    } else if (z8c_dev >= Z8C_GPIO_MIN && z8c_dev <= Z8C_GPIO_MAX) {
        iox_write(z8c_dev, z8c_reg, data);
    }
}

void z8c_sethi(uint8_t data)
{
}

uint8_t z8c_getlo()
{
    if (z8c_dev == Z8C_RTC) {
#ifdef USE_RTC
        return rtc_read(z8c_reg);
#endif
    } else if (z8c_dev >= Z8C_GPIO_MIN && z8c_dev <= Z8C_GPIO_MAX) {
        return iox_read(z8c_dev, z8c_reg);
    }
}

uint8_t z8c_gethi()
{
}