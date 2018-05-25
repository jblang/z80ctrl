/* 
   z80ctrl (https://github.com/jblang/z80ctrl)
   Copyright (c) 2018 J.B. Langston
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

/** @file diskemu.c MITS Altair 88-DISK Simulator

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

#include "util.h"
#include "bus.h"
#include "diskemu.h"
#include "ff.h"
#include "iorq.h"
#include "simhboot.h"

typedef struct _drive {
    FIL fp;
    uint8_t status;
    uint8_t track;
    uint8_t sector;
    uint8_t byte;
} drive;

#define NUMTRACKS 254ul // Altair disk has 77 but SIMH allows disk images with more
#define NUMSECTORS 32ul
#define SECTORSIZE 137ul

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
#define NUMDRIVES 26

drive drives[NUMDRIVES];
drive *selected;

uint8_t sectorbuf[SECTORSIZE+1];
uint8_t dirtysector = 0;

/**
 * Load a CPM boot sector directly from disk image mounted on drive 0
 */
int drive_bootload()
{
    FRESULT fr;
    UINT read;
    uint8_t buf[SECTORSIZE+1];
    uint16_t addr = 0;
    uint8_t sector = 0;
    uint8_t track = 0;
    uint16_t end;
    if (drives[0].status & (1 << S_MOUNTED)) {
        if ((fr = f_read(&drives[0].fp, buf, 3, &read)) != FR_OK) {
            printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
            return 0;
        }
        if (buf[0] == 0xE5 && buf[1] == 0xE5 && buf[2] == 0xE5) {
            // SIMH disks
            sector = 8;
            end = 0x5c00;
            // SIMH BIOS expects the bootloader to be there even though we don't use it
            write_mem_P(0xff00, simhboot_bin, simhboot_bin_len);
        } else {
            // Other disks
            sector = 0;
            end = buf[1] | (buf[2] << 8);
        }
        for (addr = 0; addr < 0x5c00; addr += 0x80) {
            if ((fr = f_lseek(&drives[0].fp, OFFSET(track, sector))) != FR_OK) {
                printf_P(PSTR("seek error: %S\n"), strlookup(fr_text, fr));
                return 0;
            }
            if ((fr = f_read(&drives[0].fp, buf, SECTORSIZE, &read)) != FR_OK) {
                printf_P(PSTR("read error: %S\n"), strlookup(fr_text, fr));
                return 0;
            }
            write_mem(addr, buf+3, 0x80);
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

/*  The hard disk port is 0xfd. It understands the following commands.
    1.  Reset
        ld  b,32
        ld  a,HDSK_RESET
    l:  out (0fdh),a
        dec b
        jp  nz,l
    2.  Read / write
        ; parameter block
        cmd:        db  HDSK_READ or HDSK_WRITE
        hd:         db  0   ; 0 .. 7, defines hard disk to be used
        sector:     db  0   ; 0 .. 31, defines sector
        track:      dw  0   ; 0 .. 2047, defines track
        dma:        dw  0   ; defines where result is placed in memory
        ; routine to execute
        ld  b,7             ; size of parameter block
        ld  hl,cmd          ; start address of parameter block
    l:  ld  a,(hl)          ; get byte of parameter block
        out (0fdh),a        ; send it to port
        inc hl              ; point to next byte
        dec b               ; decrement counter
        jp  nz,l            ; again, if not done
        in  a,(0fdh)        ; get result code
    3.  Retrieve Disk Parameters from controller (Howard M. Harte)
        Reads a 19-byte parameter block from the disk controller.
        This parameter block is in CP/M DPB format for the first 17 bytes,
        and the last two bytes are the lsb/msb of the disk's physical
        sector size.
        ; routine to execute
        ld   a,hdskParam    ; hdskParam = 4
        out  (hdskPort),a   ; Send 'get parameters' command, hdskPort = 0fdh
        ld   a,(diskno)
        out  (hdskPort),a   ; Send selected HDSK number
        ld   b,17
    1:  in   a,(hdskPort)   ; Read 17-bytes of DPB
        ld   (hl), a
        inc  hl
        djnz 1
        in   a,(hdskPort)   ; Read LSB of disk's physical sector size.
        ld   (hsecsiz), a
        in   a,(hdskPort)   ; Read MSB of disk's physical sector size.
        ld   (hsecsiz+1), a
*/

#define CPM_OK                  0               /* indicates to CP/M everything ok          */
#define CPM_ERROR               1               /* indicates to CP/M an error condition     */
#define CPM_EMPTY               0xe5

#define HDSK_NONE               0
#define HDSK_RESET              1
#define HDSK_READ               2
#define HDSK_WRITE              3
#define HDSK_PARAM              4

uint8_t skew[] =  { 
    0,  17, 2,  19, 4,  21, 6,  23,
    8,  25, 10, 27, 12, 29, 14, 31,
    16, 1,  18, 3,  20, 5,  22, 7,
    24, 9,  26, 11, 28, 13, 30, 15 
};

uint8_t dpb[] = {
//  SPTL, SPTH, BSH,  BLM,  EXM,  DSML, DSMH, DRML, DRMH, AL0, 
    0x20, 0x00, 0x04, 0x0F, 0x00, 0xEF, 0x01, 0xFF, 0x00, 0xF0,   
//  AL1,  CKSL, CKSH, OFFL, OFFH, PSH,  PHM,  SSL,  SSH
    0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x80, 0x00
};

#define DPBLEN 19
#define CMDLEN 6

uint8_t hdsk_command;
uint8_t hdsk_index;
uint8_t dma_disk;
uint8_t dma_sector;
uint16_t dma_track;
uint16_t dma_addr;

/**
 * Perform DMA disk read
 */
void hdsk_dma_read()
{
    UINT br;
    FRESULT fr;
    uint8_t buf[SECTORSIZE+1];
    if (dma_sector >= NUMSECTORS)
        dma_sector = 0;
    if (dma_track >= NUMTRACKS) 
        dma_track = 0;
    dma_sector = skew[dma_sector];
    //printf_P(PSTR("read disk %d sector %d track %d address %04x\n"), dma_disk, dma_sector, dma_track, dma_addr);
    if ((fr = f_lseek(&drives[dma_disk].fp, OFFSET(dma_track, dma_sector))) != FR_OK) {
        printf_P(PSTR("dma seek error: %S\n"), strlookup(fr_text, fr));
    } else if ((fr = f_read(&drives[dma_disk].fp, buf, SECTORSIZE, &br)) != FR_OK) {
        printf_P(PSTR("dma read error: %S\n"), strlookup(fr_text, fr));
    } else {
        write_mem(dma_addr, buf+3, 0x80);
    }
}

/**
 * Perform DMA disk write
 */
void hdsk_dma_write()
{
    UINT bw;
    FRESULT fr;
    uint8_t buf[SECTORSIZE+1];
    if (dma_sector >= NUMSECTORS)
        dma_sector = 0;
    if (dma_track >= NUMTRACKS) 
        dma_track = 0;
    dma_sector = skew[dma_sector];
    //printf_P(PSTR("write disk %d sector %d track %d address %04x\n"), dma_disk, dma_sector, dma_track, dma_addr);
    if ((fr = f_lseek(&drives[dma_disk].fp, OFFSET(dma_track, dma_sector))) != FR_OK) {
        printf_P(PSTR("dma seek error: %S\n"), strlookup(fr_text, fr));
    } else {
        read_mem(dma_addr, buf+3, 0x80);
        if ((fr = f_write(&drives[dma_disk].fp, buf, SECTORSIZE, &bw)) != FR_OK) {
            printf_P(PSTR("dma write error: %S\n"), strlookup(fr_text, fr));
        }        
    }
}

/**
 * Initiate command and return status read from hard drive port
 */
uint8_t hdsk_in() 
{
    uint8_t result = CPM_ERROR;
    if ((hdsk_index == CMDLEN) && ((hdsk_command == HDSK_READ) || (hdsk_command == HDSK_WRITE))) {
        if (hdsk_command == HDSK_READ)
            dma_function = &hdsk_dma_read;
        else
            dma_function = &hdsk_dma_write;
        hdsk_command = HDSK_NONE;
        hdsk_index = 0;
        result = CPM_OK;
    } else if (hdsk_command == HDSK_PARAM) {
        result = dpb[hdsk_index++];
        if (hdsk_index >= DPBLEN) {
            hdsk_command = HDSK_NONE;
            hdsk_index = 0;
        }
    }
    return result;
}

/**
 * Set up command written to hard drive port
 */
void hdsk_out(uint8_t data) 
{
    if (hdsk_command == HDSK_PARAM) {
        hdsk_index = 0;
    } else if (hdsk_command == HDSK_READ || hdsk_command == HDSK_WRITE) {
        if (hdsk_index < CMDLEN) {
            switch(hdsk_index) {
                case 0:
                    dma_disk = data;
                    hdsk_index++;
                    break;
                case 1:
                    dma_sector = data;
                    hdsk_index++;
                    break;
                case 2:
                    dma_track = data;
                    hdsk_index++;
                    break;
                case 3:
                    dma_track += (data << 8);
                    hdsk_index++;
                    break;
                case 4:
                    dma_addr = data;
                    hdsk_index++;
                    break;
                case 5:
                    dma_addr += (data << 8);
                    hdsk_index++;
                    break;
                default:
                    hdsk_command = HDSK_NONE;
                    hdsk_index = 0;
            }        } else {
            hdsk_command = HDSK_NONE;
            hdsk_index = 0;
        }
    } else {
        if ((HDSK_RESET <= data) && (data <= HDSK_PARAM)) {
            hdsk_command = data;
        } else {
            hdsk_command = HDSK_RESET;
        }
        hdsk_index = 0;
    }
}