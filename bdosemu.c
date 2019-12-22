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

/*
 * Credit to RunCPM https://github.com/MockbaTheBorg/RunCPM for showing me
 * that emulating BDOS on a FAT filesystem was possible, and for serving as
 * a sanity check to help me understand BDOS behavior when I wasn't sure
 * I understoood the BDOS assembly code.  No code has been copied though.
 */

/**
 * @file bdosemu.c BDOS disk emulation on FatFS
 */

#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bdosemu.h"
#include "iorq.h"
#include "util.h"
#include "ff.h"

// Uncomment for debug messages
//#define BDOS_DEBUG

// CP/M constants
#define RECSIZ 128L                 // # of bytes in a record (rc)
#define RECCNT 128L                 // Max # of records in an extent
#define EXTSIZ (RECSIZ*RECCNT)      // # of bytes in an extent (ex)
#define EXTCNT 32L                  // Max # of extents in a module
#define MODSIZ (EXTSIZ*EXTCNT)      // # of bytes in a module (s2)
#define MODCNT 16L                  // Max # of modules in a file
#define DEFFCB 0x5c                 // Default FCB location
#define DEFDMA 0x80                 // Default DMA buffer

// BDOS function calls
enum {
    BDOS_TERMCPM = 0,
    BDOS_OPEN = 15,
    BDOS_CLOSE = 16,
    BDOS_SFIRST = 17,
    BDOS_SNEXT = 18,
    BDOS_DELETE = 19,
    BDOS_READ = 20,
    BDOS_WRITE = 21,
    BDOS_MAKE = 22,
    BDOS_RENAME = 23,
    BDOS_ATTRIB = 30,
    BDOS_READRAND = 33,
    BDOS_WRITERAND = 34,
    BDOS_SIZE = 35,
    BDOS_RANDREC = 36,
    BDOS_WRITEZF = 40
};

const char bdos_names[] PROGMEM = {
    "P_TERMCPM\0" "C_READ\0" "C_WRITE\0" "A_READ\0" "A_WRITE\0" "L_WRITE\0" 
    "C_RAWIO\0" "A_STATIN\0" "A_STATOUT\0" "C_WRITESTR\0" "C_READSTR\0"
    "C_STAT\0" "S_BDOSVER\0" "DRV_ALLRESET\0" "DRV_SET\0" "F_OPEN\0" "F_CLOSE\0"
    "F_SFIRST\0" "F_SNEXT\0" "F_DELETE\0" "F_READ\0" "F_WRITE\0" "F_MAKE\0"
    "F_RENAME\0" "DRV_LOGINVEC\0" "DRV_GET\0" "F_DMAOFF\0" "DRV_ALLOCVEC\0"
    "DRV_SETRO\0" "DRV_ROVEC\0" "F_ATTRIB\0" "DRV_DPB\0" "F_USERNUM\0"
    "F_READRAND\0" "F_WRITERAND\0" "F_SIZE\0" "F_RANDREC\0" "DRV_RESET\0"
    "DRV_ACCESS\0" "DRV_FREE\0" "F_WRITEZF\0"
};

// BDOS return codes
enum {
    BDOS_SUCCESS = 0,
    BDOS_EOF = 1,
    BDOS_ERROR = 0xff
};

// FCB layout
typedef struct {
    uint8_t dr;
    uint8_t fn[11];
    uint8_t ex;
    uint8_t s1;
    uint8_t s2;
    uint8_t rc;
    uint8_t fatfn[13];
    uint8_t mode;
    uint16_t seq;
    uint8_t cr;
    uint8_t r0, r1, r2;
} fcb_t;

typedef struct {
    uint8_t dr;
    uint8_t fn[11];
    uint8_t ex;
    uint8_t s1;
    uint8_t s2;
    uint8_t rc;
    uint16_t al[8];
} dir_t;

// Disk Parameter Block
typedef struct {
    uint16_t spt;
    uint8_t bsh;
    uint8_t blm;
    uint8_t exm;
    uint16_t dsm;
    uint16_t drm;
    uint8_t al0;
    uint8_t al1;
    uint16_t cks;
    uint16_t off;
} dpb_t;

// Parameter mailbox format
typedef struct {
    uint16_t fcbaddr;
    uint16_t ret;
    uint8_t curdrv;
    uint16_t dmaaddr;
    uint8_t usernum;
    uint16_t alloc;
    uint16_t rodsk;
    uint16_t dlog;
} bdos_mailbox_t;

static uint16_t dma_mailbox = 0;
static uint8_t dma_command = 0;
static dma_status_t dma_status = DMA_MAILBOX_UNSET;
static bdos_mailbox_t params;

static uint16_t nextseq = 1;    // sequence numbers to uniquely identify FCBs
static uint16_t curseq = 0;

static fcb_t curfcb;            // FCB for the current command
static dir_t dirfcb;            // Directory entry returned by the last search command

static FIL fil;
static DIR dir;
static FILINFO fno;

/**
 * Get null-terminated filename.ext from fixed length FCB field
 */
void fcb_fatname(uint8_t *fatname, uint8_t *fcbname)
{
    uint8_t j = 0;
    for (uint8_t i = 0; i < 11; i++) {
        if (i == 8)
            fatname[j++] = '.';
        if (fcbname[i] != ' ')
            fatname[j++] = toupper(fcbname[i] & 0x7f);
    }
    fatname[j] = '\0';
}

/**
 * Set fixed-length FCB field from null-terminated filename.ext
 */
void fcb_setname(uint8_t *fcbname, uint8_t *fatname)
{
    uint8_t i = 0, j = 0;
    while (i < 8)
        if (fatname[j] == '.' || fatname[j] == 0)
            fcbname[i++] = ' ';
        else if (fatname[j] == '*')
            fcbname[i++] = '?';
        else
            fcbname[i++] = toupper(fatname[j++] & 0x7f);
    while (fatname[j] == '*' || fatname[j] == '.')
        j++;
    while (i < 11)
        if (fatname[j] == 0)
            fcbname[i++] = ' ';
        else if (fatname[j] == '*')
            fcbname[i++] = '?';
        else
            fcbname[i++] = toupper(fatname[j++] & 0x7f);
}

/**
 * Parse d:filename.ext argument and set FCB fields
 */
void fcb_setdrivename(uint8_t *drive, uint8_t *filename, char *arg)
{
    if (arg[1] == ':') {
        if (arg[0] >= 'a' && arg[0] <= 'p')
            *drive = arg[0] - 'a' + 1;
        else if (arg[0] >= 'A' && arg[0] <= 'P')
            *drive = arg[0] - 'A' + 1;
        else
            *drive = 0;
        fcb_setname(filename, arg+2);
    } else {
        fcb_setname(filename, arg);
    }
}

/**
 * Calculate a 32-bit offset from random fields in FCB
 */
uint32_t fcb_randoffset(fcb_t *f)
{
    return (((uint32_t)f->r2 << 16) | (f->r1 << 8) | f->r0) * RECSIZ;
}

/**
 * Calculate a 32-bit offset from sequential fields in FCB
 */
uint32_t fcb_seqoffset(fcb_t *f)
{
    return f->s2 * MODSIZ + f->ex * EXTSIZ + f->cr * RECSIZ;
}

/**
 * Set sequential fields in FCB from 32-bit offset
 */
void fcb_setseq(fcb_t *f, uint32_t offset)
{
    f->cr = (offset / RECSIZ) % RECCNT;
    f->ex = (offset / EXTSIZ) % EXTCNT;
    f->s2 = (offset / MODSIZ) % MODCNT;
}

/**
 * Set random fields in FCB from 32-bit offset
 */
uint32_t fcb_setrand(fcb_t *f, uint32_t offset)
{
    f->r0 = offset & 0xff;
    f->r1 = (offset >> 8) & 0xff;
    f->r2 = (offset >> 16) & 0xff;
}

/**
 * Check whether file matches mask using CP/M wildcard rules
 */
uint8_t fcb_match(uint8_t *mask, uint8_t *fcbname)
{
    for (uint8_t i = 0; i < 11; i++)
        if (mask[i] != '?' && toupper(mask[i]) != toupper(fcbname[i]))
            return 0;
    return 1;
}

/**
 * Print the filename from an FCB
 */
uint8_t fcb_printfn(uint8_t *fn)
{
    for (uint8_t i = 0; i < 11; i++) {
        if (i == 8)
            putchar(' ');
        if (0x20 <= (fn[i] & 0x7f) && (fn[i] & 0x7f) <= 0x7e)
            putchar(fn[i] & 0x7f);
        else
            putchar(' ');
    }
}

/**
 * Print the FAT filename from an FCB
 */
uint8_t fcb_printfatfn(uint8_t *fn)
{
    for (uint8_t i = 0; i < 12; i++) {
        if (fn[i] == 0) {
            while (i++ < 12)
                putchar(' ');
            break;            
        }
        if (fn[i] >= 20)
            putchar(fn[i]);
        else
            putchar(' ');
    }
}

/**
 * Dump the contents of an FCB
 */
uint8_t fcb_dump(fcb_t *f)
{
    printf_P(PSTR("dr filename ext   cr ex s2   r0 r1 r2   rc s1   fatfn          seq  mode\n"));
    printf_P(PSTR("%02x "), f->dr);
    fcb_printfn(f->fn);
    printf_P(PSTR("   %02x %02x %02x"), f->cr, f->ex, f->s2);
    printf_P(PSTR("   %02x %02x %02x"), f->r0, f->r1, f->r2);    
    printf_P(PSTR("   %02x %02x   "), f->rc, f->s1);
    fcb_printfatfn(f->fatfn);
    printf_P(PSTR("   %04x %02x\n"), f->seq, f->mode);
}

/**
 * Dump the contents of a directory entry
 */
uint8_t dir_dump(dir_t *d)
{
    printf_P(PSTR("dr filename ext   ex s2   rc s1   alloc\n"));
    printf_P(PSTR("%02x "), d->dr);
    fcb_printfn(d->fn);
    printf_P(PSTR("   %02x %02x"), d->ex, d->s2);
    printf_P(PSTR("   %02x %02x   "), d->rc, d->s1);
    for (uint8_t i = 0; i < 8; i++)
        printf_P(PSTR("%04x "), d->al[i]);
    putchar('\n');
}

/**
 * Log the current command and parameters
 */
void bdos_log(const char *message)
{
    printf_P(PSTR("\n%S: BDOS %d %S   fcb %04xh   dma %04xh   ret %02x\n"), message, dma_command, strlookup(bdos_names, dma_command), params.fcbaddr, params.dmaaddr, params.ret);
    if (dma_command != BDOS_SNEXT && dma_command != BDOS_TERMCPM)
        fcb_dump(&curfcb);
}

/**
 * Translate FatFS error to BDOS error and log error if appropriate
 */
uint8_t bdos_error(FRESULT fr)
{
    if (fr == FR_OK) {
        return BDOS_SUCCESS;
    } else {
        bdos_log(PSTR("Error"));
        printf_P(PSTR("FatFs error %d: %S\n"), fr, strlookup(fr_text, fr));
        return BDOS_ERROR;
    }
}

/**
 * Search for a files matching name in an FCB
 */
uint8_t bdos_search(uint8_t mode)
{
    static uint8_t mask[12];
    static uint8_t searchex;
    static uint32_t bytesleft;
    static uint16_t blockno;
    FRESULT fr;
    uint8_t buf[RECSIZ];

    // On first search, save mask and extent, then reopen directory
    if (mode == BDOS_SFIRST) {
        memcpy(mask, curfcb.fn, 11);
        mask[11] = 0;
        searchex = curfcb.ex;
        f_closedir(&dir);
        if (fr = f_opendir(&dir, ".") != FR_OK)
            return bdos_error(fr);
        memset(&dirfcb, 0, sizeof(dir_t));
        bytesleft = 0;
        blockno = 1;
    } 
    
    // Get the next file if done with current file
    if (bytesleft == 0) {    
        do {
            do {
                if (fr = f_readdir(&dir, &fno) != FR_OK)
                    return bdos_error(fr);
                if (fno.fname[0] == 0)  // indicate end of directory
                    return BDOS_ERROR;
            } while (fno.fattrib & AM_DIR); // skip directories
            fcb_setname(dirfcb.fn, fno.fname);
        } while (!fcb_match(mask, dirfcb.fn));  // check filename match

        // Translate attributes
        if (fno.fattrib & AM_RDO)
            dirfcb.fn[9] |= 0x80;
        if (fno.fattrib & AM_SYS || fno.fattrib & AM_HID)
            dirfcb.fn[10] |= 0x80;
        bytesleft = fno.fsize;

        if (searchex == '?') {
            // Start at zero if all extents requested
            dirfcb.ex = 0;
        } else {
            // Return specific extent if requested
            dirfcb.ex = searchex;
        }
        dirfcb.s2 = 0;
    } else {
        // Increment extent number
        dirfcb.ex++;
        if (dirfcb.ex >= EXTCNT) {
            dirfcb.ex = 0;
            dirfcb.s2++;
        }
    }

    // Calculate the record count in this extent
    dirfcb.rc = (bytesleft + RECSIZ-1) / RECSIZ;
    if (dirfcb.rc > RECSIZ)
        dirfcb.rc = RECSIZ;

    // Set the appropriate number of allocated blocks
    uint8_t blocks = (dirfcb.rc + 31) / 32;
    for (uint8_t i = 0; i < 8; i++) {
        if (blocks > 0) {
            dirfcb.al[i] = blockno++;
            blocks--;
        } else {
            dirfcb.al[i] = 0;
        }
    }

    // Check if more extents to return, and decrement bytes left by one extent
    if (bytesleft > EXTSIZ && searchex == '?')
        bytesleft -= EXTSIZ;
    else
        bytesleft = 0;

#ifdef BDOS_DEBUG
    printf_P(PSTR("Directory Entry:\n"));
    dir_dump(&dirfcb);
#endif

    // Only write directory entry to dma buffer for actual search command
    if (dma_command == BDOS_SFIRST || dma_command == BDOS_SNEXT) {
        // Pad extra space with empty directory entries
        memcpy(buf, &dirfcb, sizeof(dir_t));
        memset(buf+sizeof(dir_t), 0xe5, RECSIZ-sizeof(dir_t));
        // Save directory entry to DMA buffer
        mem_write(params.dmaaddr, buf, RECSIZ);
    }
    return BDOS_SUCCESS;
}

/**
 * Open the FAT file needed by the FCB if it's not already
 */
FRESULT fcb_openfil(fcb_t *fcb)
{
    if (fcb->seq == curseq)
        return FR_OK;
    f_close(&fil);
    FRESULT fr = f_open(&fil, fcb->fatfn, fcb->mode);
    if (fr == FR_OK)
        curseq = fcb->seq;
    return fr;
}

/**
 * Open file named in FCB
 */
uint8_t bdos_open()
{
    FRESULT fr;

    // Handle read-only files
    curfcb.mode = FA_READ; 
    if (!(fno.fattrib & AM_RDO))
        curfcb.mode |= FA_WRITE;

    // Command-specific behavior
    if (dma_command == BDOS_OPEN) {
        // Search for first existing file to match any wildcards
        if ((fr = bdos_search(BDOS_SFIRST)) != BDOS_SUCCESS)
            return fr;
        strncpy(curfcb.fatfn, fno.fname, 13);
        curfcb.s2 = 0;  // s2 is always zeroed by open call
    } else {
        // Otherwise make a new file
        fcb_fatname(curfcb.fatfn, curfcb.fn);
        curfcb.mode |= FA_CREATE_NEW;
        // New files start at offset 0
        fcb_setseq(&curfcb, 0); 
    }

    // assign sequence number to uniquely identify fcb
    curfcb.seq = nextseq++;
    if (nextseq == 0)
        nextseq = 1;

    // Open the file
    if ((fr = fcb_openfil(&curfcb)) != FR_OK)
        return bdos_error(fr);
    // File will no longer be new next time it's opened
    curfcb.mode &= ~FA_CREATE_NEW;

    // Write back FCB except for random fields
    mem_write(params.fcbaddr, &curfcb, sizeof(fcb_t)-3);
    return BDOS_SUCCESS;
}


/**
 * Close file associated with fcb
 */
uint8_t bdos_close()
{
    f_close(&fil);
    curseq = 0; // no currently active file
    // Clear internal indentifiers from FCB
    memset(curfcb.fatfn, 0, 16);
    // Write back FCB except for random fields
    mem_write(params.fcbaddr, &curfcb, sizeof(fcb_t)-3);
    return BDOS_SUCCESS;
}

/**
 * Read or write the file associated with the FCB
 */
uint8_t bdos_readwrite()
{
    uint32_t offset;
    uint8_t buf[RECSIZ];
    FRESULT fr;
    UINT br;

    // Try to get an active FP for this FCB
    if ((fr = fcb_openfil(&curfcb)) != FR_OK)
        return bdos_error(fr);
    // Calculate offset for access method
    if (dma_command == BDOS_READ || dma_command == BDOS_WRITE) {
        offset = fcb_seqoffset(&curfcb);
    } else {
        offset = fcb_randoffset(&curfcb); 
    }
    // Don't seek past EOF when reading
    if (offset >= f_size(&fil) && (dma_command == BDOS_READ || dma_command == BDOS_READRAND))
        return BDOS_EOF;
    // Seek to calculated offset if different from current offset
    if (offset != f_tell(&fil))
        if ((fr = f_lseek(&fil, offset)) != FR_OK)
            return bdos_error(fr);
    // Do the read or write operation
    if (dma_command == BDOS_READRAND || dma_command == BDOS_READ) {
        if ((fr = f_read(&fil, buf, RECSIZ, &br)) != FR_OK)
            return bdos_error(fr);
        // pad incomplete record with 0
        memset(buf+br, 0x0, RECSIZ-br); 
        mem_write(params.dmaaddr, buf, RECSIZ);
    } else {
        mem_read(params.dmaaddr, buf, RECSIZ);
        if ((fr = f_write(&fil, buf, RECSIZ, &br)) != FR_OK)
            return bdos_error(fr);
    }
    // Automatically increment record if in sequential mode
    if (dma_command == BDOS_READ || dma_command == BDOS_WRITE)
        offset += RECSIZ;
    fcb_setseq(&curfcb, offset);
    // Write back FCB except for random fields
    mem_write(params.fcbaddr, &curfcb, sizeof(fcb_t)-3);
    // Indicate success
    return 0;
}

/**
 * Update random fields in FCB from sequential fields
 */
uint8_t bdos_randrec()
{
    fcb_setrand(&curfcb, fcb_seqoffset(&curfcb));
    mem_write(params.fcbaddr, &curfcb, sizeof(fcb_t));
    return 0;
}

/**
 * Delete file named in FCB
 */
uint8_t bdos_delete()
{
    FRESULT fr = FR_OK;
    // Search for files matching wildcards
    uint8_t ret = bdos_search(BDOS_SFIRST);
    while (!ret) {
        if (fr = f_unlink(fno.fname) != FR_OK)
            return bdos_error(fr);
        ret = bdos_search(BDOS_SNEXT);
    }
    return bdos_error(fr);
}

/**
 * Rename file named in fCB
 */
uint8_t bdos_rename()
{
    uint8_t fatfn[13], fatfn2[13];
    fcb_fatname(fatfn, curfcb.fn);
    fcb_fatname(fatfn2, curfcb.fatfn+1);
    return bdos_error(f_rename(fatfn, fatfn2));
}

/**
 * Compute file size
 */
uint8_t bdos_size()
{
    uint8_t ret;
    if (ret = bdos_search(BDOS_SFIRST))
        return ret;
    fcb_setrand(&curfcb, (fno.fsize + RECSIZ-1)/ RECSIZ);
    mem_write(params.fcbaddr, &curfcb, sizeof(fcb_t));
}

/**
 * Emulate a BDOS curdrv function using DMA
 */
void bdos_dma_execute()
{
    // Get DMA parameters
    mem_read(dma_mailbox, &params, sizeof(bdos_mailbox_t));

    // Get the specified FCB
    if (dma_command != BDOS_SNEXT) {
        mem_read(params.fcbaddr, &curfcb, sizeof(fcb_t));
    }
#ifdef BDOS_DEBUG
    bdos_log(PSTR("Before"));
#endif

    switch (dma_command) {
        case BDOS_TERMCPM:
            f_close(&fil);
            break;
        case BDOS_OPEN:
        case BDOS_MAKE:
            params.ret = bdos_open();
            break;
        case BDOS_CLOSE:
            params.ret = bdos_close();
            break;
        case BDOS_SFIRST:
        case BDOS_SNEXT:
            params.ret = bdos_search(dma_command);
            break;
        case BDOS_READ:
        case BDOS_WRITE:
        case BDOS_READRAND:
        case BDOS_WRITERAND:
        case BDOS_WRITEZF:
            params.ret = bdos_readwrite();
            break;
        case BDOS_DELETE:
            params.ret = bdos_delete();
            break;
        case BDOS_RENAME:
            params.ret = bdos_rename();
            break;
        case BDOS_ATTRIB:
            break; // currently unimplemented
        case BDOS_SIZE:
            params.ret = bdos_size();
            break;
        case BDOS_RANDREC:
            params.ret = bdos_randrec();
            break;
        default:
            params.ret = bdos_error(FR_INVALID_PARAMETER);
            break;
    }

#ifdef BDOS_DEBUG
    bdos_log(PSTR("After"));
#endif

    // Write back return values
    mem_write(dma_mailbox, &params, sizeof(bdos_mailbox_t));
}

/**
 * Initialize default FCB and command tail
 */
void bdos_init(int argc, char *argv[])
{
    char comtail[256];
    fcb_t deffcb;

    // Set up default FCB with filenames on command line
    memset(&deffcb, 0, sizeof(fcb_t));
    if (argc >= 2)
        fcb_setdrivename(&deffcb.dr, deffcb.fn, argv[1]);
    else 
        memset(deffcb.fn, ' ', 11);
    if (argc >= 3)
        fcb_setdrivename(deffcb.fatfn, deffcb.fatfn+1, argv[2]);
    else
        memset(deffcb.fatfn+1, ' ', 11);
    mem_write(DEFFCB, &deffcb, sizeof(fcb_t));
    memset(comtail, ' ', 256);

    // Save the entire command line tail to DMA buffer
    char *next = comtail + 1;
    for (int i = 1; i < argc; i++) {
        strcpy(next, argv[i]);
        next += strlen(argv[i]);
        *next++ = ' ';
    }
    comtail[0] = (uint8_t)(next - comtail - 1);
    mem_write(DEFDMA, comtail, RECSIZ);
}

/**
 * Reset the DMA parameter address
 */
uint8_t bdos_dma_reset()
{
    dma_function = NULL;
    dma_status = DMA_MAILBOX_UNSET;
    dma_mailbox = 0;
    return 0xa5;
}

/**
 * Queue a DMA command for execution
 */
void bdos_dma_command(uint8_t data) 
{
    switch (dma_status) {
        case DMA_MAILBOX_UNSET:
            dma_mailbox = data;
            dma_status = DMA_MAILBOX_HALFSET;
            break;
        case DMA_MAILBOX_HALFSET:
            dma_mailbox |= (data << 8);
            dma_status = DMA_MAILBOX_SET;
            break;
        case DMA_MAILBOX_SET:
            dma_command = data;
            dma_function = bdos_dma_execute;
            break;
        default:
            dma_function = NULL;
    }
}