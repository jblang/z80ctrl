/* diskemu.c: MITS Altair 88-DISK Simulator

   Copyright (c) 2018 J.B. Langston
   Copyright (c) 1997-2010, Charles E. Owen

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

    The 88_DISK is a 8-inch floppy controller which can control up
    to 16 daisy-chained Pertec FD-400 hard-sectored floppy drives.
    Each diskette has physically 77 tracks of 32 137-byte sectors
    each.

    The controller is interfaced to the CPU by use of 3 I/O addreses,
    standardly, these are device numbers 8H, 9H, and 0AH.

    Address     Mode    Function
    -------             ----    --------

        08H             Out             Selects and enables Controller and Drive
        08H             In              Indicates status of Drive and Controller
        09H             Out             Controls Disk Function
        09H             In              Indicates current sector position of disk
        0AH             Out             Write data
        0AH             In              Read data

    Drive Select Out (Device 8H OUT):

    +---+---+---+---+---+---+---+---+
    | C | X | X | X |   Device      |
    +---+---+---+---+---+---+---+---+

    C = If this bit is 1, the disk controller selected by 'device' is
        cleared. If the bit is zero, 'device' is selected as the
        device being controlled by subsequent I/O operations.
    X = not used
    Device = value zero thru 15, selects drive to be controlled.

    Drive Status In (Device 8H IN):

    +---+---+---+---+---+---+---+---+
    | R | Z | I | X | X | H | M | W |
    +---+---+---+---+---+---+---+---+

    W - When 0, write circuit ready to write another byte.
    M - When 0, head movement is allowed
    H - When 0, indicates head is loaded for read/write
    X - not used (will be 0)
    I - When 0, indicates interrupts enabled (not used this simulator)
    Z - When 0, indicates head is on track 0
    R - When 0, indicates that read circuit has new byte to read

    Drive Control (Device 9H OUT):

    +---+---+---+---+---+---+---+---+
    | W | C | D | E | U | H | O | I |
    +---+---+---+---+---+---+---+---+

    I - When 1, steps head IN one track
    O - When 1, steps head OUT out track
    H - When 1, loads head to drive surface
    U - When 1, unloads head
    E - Enables interrupts (ignored this simulator)
    D - Disables interrupts (ignored this simulator)
    C - When 1 lowers head current (ignored this simulator)
    W - When 1, starts Write Enable sequence:   W bit on device 10
        (see above) will go 1 and data will be read from port 12
        until 137 bytes have been read by the controller from
        that port.  The W bit will go off then, and the sector data
        will be written to disk. Before you do this, you must have
        stepped the track to the desired number, and waited until
        the right sector number is presented on device 11 IN, then
        set this bit.

    Sector Position (Device 9H IN):

    As the sectors pass by the read head, they are counted and the
    number of the current one is available in this register.

    +---+---+---+---+---+---+---+---+
    | X | X |  Sector Number    | T |
    +---+---+---+---+---+---+---+---+

    X = Not used
    Sector number = binary of the sector number currently under the
        head, 0-31.
    T = Sector True, is a 1 when the sector is positioned to read or
        write.

*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "diskemu.h"
#include "ff.h"

typedef struct _drive {
    FIL fp;
    uint8_t mounted;
    uint8_t status;
    uint8_t track;
    uint8_t sector;
    uint8_t byte;
} drive;

#define NUMDRIVES 16
#define NUMTRACKS 77ul
#define NUMSECTORS 32ul
#define SECTORSIZE 137ul

// Drive deselect bit
#define DESELECT 7

// Disk status bits
#define S_READRDY 7
#define S_TRACK0 6
#define S_INTENABLE 5
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

drive drives[NUMDRIVES];
drive *selected;

uint8_t sectorbuf[SECTORSIZE+1];
uint8_t dirtysector = 0;

void drive_unmount(uint8_t drv) 
{
    FRESULT fr;
    if ((fr = f_close(&drives[drv].fp)) != FR_OK) {
        printf_P(PSTR("error unmounting disk: %d"), fr);
    }
    drives[drv].mounted = 0;
}

void drive_mount(uint8_t drv, char *filename) 
{
    FRESULT fr;
    if (drives[drv].mounted)
        drive_unmount(drv);
    if ((fr = f_open(&drives[drv].fp, filename, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK) {
        printf_P(PSTR("error mounting disk: %d"), fr);
        return;
    }
    drives[drv].mounted = 1;
}

void write_sector(void) 
{
    FSIZE_t ofs;
    FRESULT fr;
    UINT bw;
    uint8_t i;

    if (!selected)
        return;

    for (i = selected->byte; i < SECTORSIZE; i++)
        sectorbuf[i] = 0;

    ofs = selected->track * NUMSECTORS * SECTORSIZE;
    ofs += selected->sector * SECTORSIZE;

    //printf("seeking track %d sector %d offset %u\n", selected->track, selected->sector, ofs);
    if ((fr = f_lseek(&selected->fp, ofs)) != FR_OK) {
        printf_P(PSTR("seek error: %d\n"), fr);
    } else {
        //printf("writing... ");
        if ((fr = f_write(&selected->fp, sectorbuf, SECTORSIZE, &bw)) != FR_OK) {
            printf_P(PSTR("write error: %d\n"), fr);
        }
        //printf("%d bytes\n", bw);
    }
    selected->status &= ~(1 << S_WRITERDY);
    selected->byte = 0xff;
    dirtysector = 0;
}

void drive_select(uint8_t newdrv) 
{
    //printf("selecting drive %d\n", newdrv);
    if (dirtysector)
        write_sector();
    if (selected) {
        selected->status = 0;
        selected->sector = 0xff;
        selected->byte = 0xff;
    }
    if (newdrv & (1 << DESELECT)) {
        selected = NULL;
    } else {
        selected = &drives[newdrv & 0xf];
        if (selected->mounted) {
            selected->status = (1 << S_HEADMOVE) | (1 << S_ENABLED);
            if (selected->track == 0) {
                selected->status |= (1 << S_TRACK0);
            }
        } else {
            selected->status = 0;
        }
        selected->sector = 0xff;
        selected->byte = 0xff;
    }
}

uint8_t drive_status() 
{
    if (selected) {
        //printf("returning status %x\n", selected->status);
        return ~selected->status;
    } else {
        printf("drive status error: no drive selected\n");
        return 0xFF;
    }
}

void drive_control(uint8_t cmd) 
{
    if (!selected) {
        printf("drive control error: no drive selected");
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
        //printf("stepping in: track %d\n", selected->track);
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
        //printf("stepping out: track %d\n", selected->track);
    }

    if (dirtysector)
        write_sector();

    if (cmd & (1 << C_LOAD)) {
        //printf("loading head\n");
        selected->status |= (1 << S_HEADLOAD) | (1 << S_READRDY);
    }

    if (cmd & (1 << C_UNLOAD)) {
        //printf("unloading head\n");
        selected->status &= ~((1 << S_HEADLOAD) | (1 << S_READRDY));
        selected->sector = 0xff;
        selected->byte = 0xff;
    }

    //if (cmd & (1 << C_INTENABLE))
    //if (cmd & (1 << C_INTDISABLE))
    //if (cmd & (1 << C_LOWER))

    if (cmd & (1 << C_WRITE)) {
        //printf("preparing write\n");
        selected->byte = 0;
        selected->status |= (1 << S_WRITERDY);
    }
}

uint8_t drive_sector(void) 
{
    if (!selected) {
        printf("drive sector error: no drive selected\n");
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
        //printf("sector %d\n", selected->sector);
        return (selected->sector << 1);
    } else {
        return 0;
    }
}

void drive_write(uint8_t data) 
{
    if (!selected) {
        printf("drive write error: no drive selected\n");
        return;
    }

    //printf("writing %02x to byte %d of sector %d, track %d\n", data, selected->byte, selected->sector, selected->track);
    sectorbuf[selected->byte] = data;

    if (selected->byte < SECTORSIZE) {
        dirtysector = 1;
        selected->byte++;
    } else {
        write_sector();
    }
}

uint8_t drive_read(void) 
{
    FSIZE_t ofs;
    FRESULT fr;
    UINT br;
    uint8_t i;

    if (!selected) {
        printf("drive read error: no drive selected\n");
        return;
    }

    if ((i = selected->byte) < SECTORSIZE) {
        //printf("reading %02x from byte %d of sector %d, track %d\n", sectorbuf[i], selected->byte, selected->sector, selected->track);
        selected->byte++;
        return sectorbuf[i];
    } else {
        ofs = selected->track * NUMSECTORS * SECTORSIZE;
        ofs += selected->sector * SECTORSIZE;
        //printf("seeking track %d sector %d offset %u\n", selected->track, selected->sector, ofs);
        if ((fr = f_lseek(&selected->fp, ofs)) != FR_OK) {
            printf_P(PSTR("seek error: %d\n"), fr);
        } else {
            //printf("reading... ");
            if ((fr = f_read(&selected->fp, sectorbuf, SECTORSIZE, &br)) != FR_OK) {
                printf_P(PSTR("read error: %d\n"), fr);
            }
            //printf("%u bytes read\n", br);
        }
        //printf("reading %02x from byte %d of sector %d, track %d\n", sectorbuf[0], 0, selected->sector, selected->track);
        selected->byte = 1;
        return sectorbuf[0];
    }
}