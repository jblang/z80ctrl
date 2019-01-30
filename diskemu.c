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

#include "util.h"
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

drive drives[NUMDRIVES];
drive *selected;

uint8_t sectorbuf[SECTORSIZE+1];
uint8_t dirtysector = 0;

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
    if ((fr = f_close(&drives[drv].fp)) != FR_OK) {
        printf_P(PSTR("error unmounting disk: %S\n"), strlookup(fr_text, fr));
    }
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
    if ((fr = f_open(&drives[drv].fp, filename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK) {
        printf_P(PSTR("error mounting disk: %S"), strlookup(fr_text, fr));
        return;
    }
    drives[drv].status |= 1 << S_MOUNTED;
    if ((fr = f_read(&drives[drv].fp, buf, 3, &br)) != FR_OK) {
        printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
        drives[drv].format = DISK_FORMAT_UNKNOWN;
    }
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

    if ((fr = f_lseek(&selected->fp, OFFSET(selected->track, selected->sector))) != FR_OK) {
        printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
    } else {
        if ((fr = f_write(&selected->fp, sectorbuf, SECTORSIZE, &bw)) != FR_OK) {
            printf_P(PSTR("write error: %S\n"), strlookup(fr_text, fr));
        }
    }
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

    if ((i = selected->byte) < SECTORSIZE) {
        selected->byte++;
        return sectorbuf[i];
    } else {
        if ((fr = f_lseek(&selected->fp, OFFSET(selected->track, selected->sector))) != FR_OK) {
            printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
        } else {
            if ((fr = f_read(&selected->fp, sectorbuf, SECTORSIZE, &br)) != FR_OK) {
                printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
            }
        }
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
            mem_write_P(0xff00, simhboot_bin, simhboot_bin_len);
        uint8_t track = 0;
        uint8_t sector = drives[0].bootstart;
        for (uint16_t addr = 0; addr < 0x5c00; addr += 0x80) {
            if ((fr = f_lseek(&drives[0].fp, OFFSET(track, sector))) != FR_OK) {
                printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
                return 0;
            }
            if ((fr = f_read(&drives[0].fp, buf, SECTORSIZE, &read)) != FR_OK) {
                printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
                return 0;
            }
            mem_write(addr, buf+3, 0x80);
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

#define DMA_DRIVE 0
#define DMA_TRACK DMA_DRIVE+1
#define DMA_STATUS DMA_TRACK+2
#define DMA_BUFFER DMA_STATUS+1
#define DMA_DPB DMA_BUFFER+TRACKSIZE
#define DMA_SKEW DMA_DPB+DPBSIZE

#define DMA_OK 0
#define DMA_INVALID_DRIVE 1
#define DMA_INVALID_TRACK 2
#define DMA_UNMOUNTED_DRIVE 3
#define DMA_DRIVE_ERROR 4

#define DMA_ADDR_UNSET 0
#define DMA_ADDR_HALFSET 1
#define DMA_ADDR_SET 2

#define DMA_CMD_RESET 0
#define DMA_CMD_DPB 1
#define DMA_CMD_READ 2
#define DMA_CMD_WRITE 3

uint16_t dma_addr = 0;
uint8_t dma_status = DMA_ADDR_UNSET;

/**
 * Get status of DMA address configuration
 */
uint8_t drive_dma_status()
{
    return dma_status;
}

uint8_t altair_dpb[] = {
    0x20, 0x00, // SPT
    0x04,       // BSH
    0x0F,       // BLM
    0x00,       // EXM
    0x95, 0x00, // DSM
    0x3F, 0x00, // DRM
    0xC0, 0x00, // AL0, AL1
    0x10, 0x00, // CKS
    0x06, 0x00, // OFF
    0x00,       // PSH
    0x00        // PHM
};

uint8_t altair_skew[] = {
     1,  9, 17, 25,  3, 11, 19, 27,
     5, 13, 21, 29,  7, 15, 23, 31,
     2, 10, 18, 26,  4, 12, 20, 28,
     6, 14, 22, 30,  8, 16, 24, 32
};

uint8_t simh_dpb[] = {
    0x20, 0x00, // SPT
    0x04,       // BSH
    0x0F,       // BLM
    0x00,       // EXM
    0xEF, 0x01, // DSM
    0xFF, 0x00, // DRM
    0xF0, 0x00, // AL0, AL1
    0x00, 0x00, // CKS
    0x02, 0x00, // OFF
    0x00,       // PSH
    0x00        // PHM
};

uint8_t simh_skew[] = { 
     1, 18,  3, 20,  5, 22,  7, 24,
	 9, 26, 11, 28, 13, 30, 15, 32,
	17,  2, 19,  4, 21,  6, 23,  8,
	25, 10, 27, 12, 29, 14, 31, 16
};

uint8_t *dpbs[] = {
    NULL,
    simh_dpb,
    altair_dpb
};

uint8_t *skews[] = {
    NULL,
    simh_skew,
    altair_skew
};

/**
 * Load the currently selected drive's disk parameter block
 */
void drive_dma_setdpb()
{
    uint8_t buf[SKEWSIZE+DPBSIZE];
    mem_read_bare(dma_addr, buf, 1);
    uint8_t drive = buf[DMA_DRIVE];
    uint8_t status = DMA_OK;
    printf("setting dbp for %d; ", drive);
    if (drive >= NUMDRIVES)
        status = DMA_INVALID_DRIVE;
    else if (!(drives[drive].status & (1 << S_MOUNTED)))
        status = DMA_UNMOUNTED_DRIVE;
    else {
        mem_write_bare(dma_addr+DMA_DPB, dpbs[drives[drive].format], DPBSIZE);
        mem_write_bare(dma_addr+DMA_SKEW, skews[drives[drive].format], SKEWSIZE);
    }
    mem_write_bare(dma_addr+DMA_STATUS, &status, 1);
    printf("status %d\n", status);
}

/**
 * Perform DMA disk read
 */
void drive_dma_read()
{
    UINT br;
    FRESULT fr;
    uint8_t buf[TRACKSIZE+1];   // track + status
    mem_read_bare(dma_addr, buf, 3);
    uint8_t drive = buf[DMA_DRIVE];
    uint16_t track = buf[DMA_TRACK] | (buf[DMA_TRACK+1] << 8);
    uint8_t status = DMA_OK;
    if (drive >= NUMDRIVES)
        status = DMA_INVALID_DRIVE;
    else if (track >= NUMTRACKS)
        status = DMA_INVALID_TRACK;
    else if (!(drives[drive].status & (1 << S_MOUNTED)))
        status = DMA_UNMOUNTED_DRIVE;
    else {
        //printf_P(PSTR("read disk %d sector %d track %d address %04x\n"), dma_disk, dma_sector, dma_track, dma_addr);
        if ((fr = f_lseek(&drives[drive].fp, OFFSET(track, 0))) != FR_OK) {
            printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
            status = DMA_DRIVE_ERROR;
        } else if ((fr = f_read(&drives[drive].fp, buf+1, TRACKSIZE, &br)) != FR_OK) {
            printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
            status = DMA_DRIVE_ERROR;
        }
    }
    if (status == DMA_OK) {
        buf[0] = status;
        mem_write_bare(dma_addr+DMA_STATUS, buf, TRACKSIZE+1);
    } else {
        mem_write_bare(dma_addr+DMA_STATUS, &status, 1);
    }
}

/**
 * Perform DMA disk write
 */
void drive_dma_write()
{
    UINT bw;
    FRESULT fr;
    uint8_t buf[TRACKSIZE];
    mem_read_bare(dma_addr, buf, 3);
    uint8_t drive = buf[DMA_DRIVE];
    uint16_t track = buf[DMA_TRACK] | (buf[DMA_TRACK+1] << 8);
    uint8_t status = DMA_OK;
    if (drive >= NUMDRIVES)
        status = DMA_INVALID_DRIVE;
    else if (track >= NUMTRACKS)
        status = DMA_INVALID_TRACK;
    else if (!(drives[0].status & (1 << S_MOUNTED)))
        status = DMA_UNMOUNTED_DRIVE;
    else {
        //printf_P(PSTR("write disk %d sector %d track %d address %04x\n"), dma_disk, dma_sector, dma_track, dma_addr);
        if ((fr = f_lseek(&drives[drive].fp, OFFSET(track, 0))) != FR_OK) {
            status = DMA_DRIVE_ERROR;
            printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
        } else {
            mem_read_bare(dma_addr+DMA_BUFFER, buf, TRACKSIZE);
            if ((fr = f_write(&drives[drive].fp, buf, TRACKSIZE, &bw)) != FR_OK) {
                status = DMA_DRIVE_ERROR;
                printf_P(PSTR("write error: %S\n"), strlookup(fr_text, fr));
            }
        }
    }
    mem_write_bare(dma_addr+DMA_STATUS, &status, 1);
}

/**
 * Queue a DMA command for execution
 */
void drive_dma_command(uint8_t data) 
{
    switch (dma_status) {
        case DMA_ADDR_UNSET:
            dma_addr = data;
            dma_status = DMA_ADDR_HALFSET;
            break;
        case DMA_ADDR_HALFSET:
            dma_addr |= (data << 8);
            dma_status = DMA_ADDR_SET;
            break;
        case DMA_ADDR_SET:
            switch(data) {
                case DMA_CMD_RESET:
                    dma_status = DMA_ADDR_UNSET;
                    dma_addr = 0;
                    dma_function = NULL;
                    break;
                case DMA_CMD_DPB:
                    dma_function = drive_dma_setdpb;
                    break;
                case DMA_CMD_READ:
                    dma_function = drive_dma_read;
                    break;
                case DMA_CMD_WRITE:
                    dma_function = drive_dma_write;
                    break;
                default:
                    dma_function = NULL;
            } 
            break;
        default:
            dma_function = NULL;
    }
}