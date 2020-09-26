/* 
   z80ctrl (https://github.com/jblang/z80ctrl)
   Copyright (c) 2018-2019 J.B. Langston
   Copyright (c) 1997-2010, Charles E. Owen
   Copyright (c) 2002-2014, Peter Schorn

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   ROBERT M SUPNIK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of Charles E. Owen shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from Charles E. Owen.
*/

/**
 *  @file diskemu.c MITS Altair 88-DISK Simulator
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "ffwrap.h"
#include "bus.h"
#include "diskemu.h"
#include "ff.h"
#include "iorq.h"
#include "simhboot.h"

#define DISK_FORMAT_UNKNOWN 0
#define DISK_FORMAT_SIMH 1
#define DISK_FORMAT_ALTAIR 2

typedef struct _drive {
    FIL fp;
    uint8_t status;
    uint8_t track;
    uint8_t sector;
    uint8_t byte;
    uint8_t format;
    uint16_t bootstart;
    uint16_t bootend;
} drive;

#define NUMTRACKS 254ul // Altair disk has 77 but SIMH allows disk images with more
#define NUMSECTORS 32ul
#define SECTORSIZE 137ul
#define TRACKSIZE NUMSECTORS*SECTORSIZE
#define DPBSIZE 17
#define SKEWSIZE 32

#define OFFSET(track, sector) (((track) * NUMSECTORS * SECTORSIZE) + ((sector) * SECTORSIZE))

// Drive deselect bit
#define DESELECT 7

// Disk status bits
#define S_READRDY 7
#define S_TRACK0 6
#define S_INTENABLE 5
#define S_MOUNTED 4
#define S_ENABLED 3
#define S_HEADLOAD 2
#define S_HEADMOVE 1
#define S_WRITERDY 0

// Disk command bits
#define C_WRITE 7
#define C_LOWER 6
#define C_INTENABLE 5
#define C_INTDISABLE 4
#define C_UNLOAD 3
#define C_LOAD 2
#define C_STEPOUT 1
#define C_STEPIN 0

// Number of emulated disk drives
#define NUMDRIVES 8

static drive drives[NUMDRIVES];
static drive *selected;

static uint8_t sectorbuf[SECTORSIZE+1];
static uint8_t dirtysector = 0;

/**
 * Unmount a disk image
 */
void drive_unmount(uint8_t drv) 
{
    if (drv >= NUMDRIVES) {
        printf_P(PSTR("error: valid drive numbers are 0-%d\n"), NUMDRIVES-1);
        return;
    }
    FRESULT fr;
    file_close(&drives[drv].fp);
    drives[drv].status &= ~(1 << S_MOUNTED);
}

/**
 * Mount a disk image
 */
void drive_mount(uint8_t drv, char *filename) 
{
    UINT br;
    uint8_t buf[3];
    if (drv >= NUMDRIVES) {
        printf_P(PSTR("error: valid drive numbers are 0-%d\n"), NUMDRIVES-1);
        return;
    }
    FRESULT fr;
    if (drives[drv].status & (1 << S_MOUNTED))
        drive_unmount(drv);
    if ((fr = file_open(&drives[drv].fp, NULL, filename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
        return;
    drives[drv].status |= 1 << S_MOUNTED;
    if ((fr = file_read(&drives[drv].fp, buf, 3, &br)) != FR_OK)
        drives[drv].format = DISK_FORMAT_UNKNOWN;
    if (buf[0] == 0xE5 && buf[1] == 0xE5 && buf[2] == 0xE5) {
        // SIMH disks
        drives[drv].format = DISK_FORMAT_SIMH;
        drives[drv].bootstart = 8;
        drives[drv].bootend = 0x5c00;
    } else {
        // Altair disks
        drives[drv].format = DISK_FORMAT_ALTAIR;
        drives[drv].bootstart = 0;
        drives[drv].bootend = buf[1] | (buf[2] << 8);
    }
}

/**
 * Write the current sector to SD card
 */
void write_sector(void) 
{
    FRESULT fr;
    UINT bw;
    uint8_t i;

    if (!selected)
        return;

    for (i = selected->byte; i < SECTORSIZE; i++)
        sectorbuf[i] = 0;

    if ((fr = file_seek(&selected->fp, OFFSET(selected->track, selected->sector))) == FR_OK)
        file_write(&selected->fp, sectorbuf, SECTORSIZE, &bw);
    selected->status &= ~(1 << S_WRITERDY);
    selected->byte = 0xff;
    dirtysector = 0;
}

/**
 * Select the active drive
 */
void drive_select(uint8_t newdrv) 
{
    if (dirtysector)
        write_sector();
    if (selected) {
        selected->status &= (1 << S_MOUNTED);
        selected->sector = 0xff;
        selected->byte = 0xff;
    }
    if (newdrv >= NUMDRIVES) {
        selected = NULL;
    } else {
        selected = &drives[newdrv];
        if ((selected->status) & (1 << S_MOUNTED)) {
            selected->status |= (1 << S_HEADMOVE) | (1 << S_ENABLED);
            if (selected->track == 0) {
                selected->status |= (1 << S_TRACK0);
            }
        } else {
            selected->status &= (1 << S_MOUNTED);
        }
        selected->sector = 0xff;
        selected->byte = 0xff;
    }
}

/**
 * Get the current drive's status
 */
uint8_t drive_status() 
{
    if (selected) {
        return ~selected->status;
    } else {
        printf_P(PSTR("drive status error: no drive selected\n"));
        return 0xFF;
    }
}

/**
 * Update the current drive's control register
 */
void drive_control(uint8_t cmd) 
{
    if (!selected) {
        printf_P(PSTR("drive control error: no drive selected\n"));
        return;
    }

    if (cmd & (1 << C_STEPIN)) {
        if (selected->track < NUMTRACKS-1)
            selected->track++;
        selected->status &= ~(1 << S_TRACK0);
        if (dirtysector)
            write_sector();
        selected->sector = 0xff;
        selected->byte = 0xff;
    }

    if (cmd & (1 << C_STEPOUT)) {
        if (selected->track > 0)
            selected->track--;
        if (selected->track == 0)
            selected->status |= (1 << S_TRACK0);
        if (dirtysector)
            write_sector();
        selected->sector = 0xff;
        selected->byte = 0xff;
    }

    if (dirtysector)
        write_sector();

    if (cmd & (1 << C_LOAD)) {
        selected->status |= (1 << S_HEADLOAD) | (1 << S_READRDY);
    }

    if (cmd & (1 << C_UNLOAD)) {
        selected->status &= ~((1 << S_HEADLOAD) | (1 << S_READRDY));
        selected->sector = 0xff;
        selected->byte = 0xff;
    }

    if (cmd & (1 << C_WRITE)) {
        selected->byte = 0;
        selected->status |= (1 << S_WRITERDY);
    }
}

/**
 * Get the current drive's active sector
 */
uint8_t drive_sector(void) 
{
    if (!selected) {
        printf_P(PSTR("drive sector error: no drive selected\n"));
        return 0;
    }
    if (dirtysector)
        write_sector();
    if (selected->status & (1 << S_HEADLOAD)) {
        selected->sector++;
        if (selected->sector >= NUMSECTORS) {
            selected->sector = 0;
        }
        selected->byte = 0xff;
        return (selected->sector << 1);
    } else {
        return 0;
    }
}

/**
 * Write a byte to the current drive
 */
void drive_write(uint8_t data) 
{
    if (!selected) {
        printf_P(PSTR("drive write error: no drive selected\n"));
        return;
    }

    sectorbuf[selected->byte] = data;

    if (selected->byte < SECTORSIZE) {
        dirtysector = 1;
        selected->byte++;
    } else {
        write_sector();
    }
}

/**
 * Read a byte from the current drive
 */
uint8_t drive_read(void) 
{
    FRESULT fr;
    UINT br;
    uint8_t i;

    if (!selected) {
        printf_P(PSTR("drive read error: no drive selected\n"));
        return 0;
    }

    //printf("read from track %d sector %d\n", selected->track, selected->sector);
    if ((i = selected->byte) < SECTORSIZE) {
        selected->byte++;
        return sectorbuf[i];
    } else {
        if ((fr = file_seek(&selected->fp, OFFSET(selected->track, selected->sector))) == FR_OK)
            file_read(&selected->fp, sectorbuf, SECTORSIZE, &br);
        selected->byte = 1;
        return sectorbuf[0];
    }
}


/**
 * Load a CPM boot sector directly from disk image mounted on drive 0
 */
int drive_bootload()
{
    FRESULT fr;
    UINT read;
    uint8_t buf[SECTORSIZE+1];
    if (drives[0].status & (1 << S_MOUNTED)) {
        // SIMH BIOS expects the bootloader to be there even though we don't use it
        if (drives[0].format == DISK_FORMAT_SIMH)
            mem_write_banked_P(0xff00, simhboot_bin, simhboot_bin_len);
        uint8_t track = 0;
        uint8_t sector = drives[0].bootstart;
        for (uint16_t addr = 0; addr < 0x5c00; addr += 0x80) {
            if ((fr = file_seek(&drives[0].fp, OFFSET(track, sector))) != FR_OK)
                return 0;
            if ((fr = file_read(&drives[0].fp, buf, SECTORSIZE, &read)) != FR_OK)
                return 0;
            mem_write_banked(addr, buf+3, 0x80);
            sector += 2;
            if (sector == NUMSECTORS)
                sector = 1;
            else if (sector > NUMSECTORS) {
                sector = 0;
                track++;
            }
        }
    } else {
        printf_P(PSTR("boot error: drive 0 not mounted\n"));
        return 0;
    }
    return 1;
}
