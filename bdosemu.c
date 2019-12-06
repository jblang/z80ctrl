#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bdosemu.h"
#include "iorq.h"
#include "util.h"
#include "ff.h"

//#define BDOS_DEBUG

#define NOFILES 16
#define NODRIVES 4

// CP/M constants
#define RECSIZ 128L                 // # of bytes in a record (rc)
#define RECCNT 128L                 // Max # of records in an extent
#define EXTSIZ (RECSIZ*RECCNT)      // # of bytes in an extent (ex)
#define EXTCNT 32L                  // Max # of extents in a module
#define MODSIZ (EXTSIZ*EXTCNT)      // # of bytes in a module (s2)
#define MODCNT 16L                  // Max # of modules in a file
#define DEFFCB 0x5c                 // Default FCB location
#define DEFDMA 0x80                 // Default DMA buffer

enum {
    FP_NEW,
    FP_FIND,
    FP_REMOVE
};

enum {
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



#define BDOS_ERROR 0xff

typedef struct {
    uint8_t dr;
    uint8_t fn[11];
    uint8_t ex;
    uint8_t s1;
    uint8_t s2;
    uint8_t rc;
    uint8_t dr2;
    uint8_t fn2[11];
    uint16_t fcbaddr;
    uint16_t filaddr;
    uint8_t cr;
    uint8_t r0, r1, r2;
} fcb_t;

typedef struct {
    uint8_t fcbaddr;
    FIL fil;
} filwrap_t;

typedef struct {
    uint16_t fcbaddr;
    uint16_t ret;
    uint8_t curdrv;
    uint16_t dmaaddr;
} bdos_mailbox_t;

static uint16_t dma_mailbox = 0;
static uint8_t dma_command = 0;
static dma_status_t dma_status = DMA_MAILBOX_UNSET;


bdos_mailbox_t params;
fcb_t reqfcb;

/**
 * Return FatFS file object associated with FCB address
 */
FIL *fcb_getfp(uint8_t mode, uint16_t fcbaddr)
{
    static FIL fil;
    static uint16_t lastfcb = 0;
    static uint16_t fpaddr = 0;
    uint16_t base = dma_mailbox + sizeof(bdos_mailbox_t);
    uint16_t findfcb = (mode == FP_NEW) ? 0 : fcbaddr;
    uint16_t fcbaddrs[NOFILES];

    if (mode == FP_FIND && findfcb == lastfcb) {
        return &fil;
    }

    if (fpaddr != 0) {
        mem_write_bare(fpaddr, &fil, sizeof(FIL));
    }

    mem_read_bare(base, fcbaddrs, NOFILES*2);
    
    for (uint8_t i = 0; i < NOFILES; i++) {
        if (findfcb == fcbaddrs[i]) {
            fpaddr = base + NOFILES*2 + sizeof(FIL)*i;

            if (mode == FP_NEW) {
                mem_write_bare(base + i*2, &fcbaddr, 2);
            } else {
                mem_read_bare(fpaddr, &fil, sizeof(FIL));
            }

            if (mode == FP_REMOVE) {
                fpaddr = 0;
                fcbaddr = 0;
                mem_write_bare(base + i * 2, &fcbaddr, 2);
            }

            lastfcb = fcbaddr;
            return &fil;
        }
    }

    return NULL;
}

/**
 * Get null-terminated filename.ext from fixed length FCB field
 */
void fcb_getname(uint8_t *fcbname, uint8_t *fatname)
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
void fcb_setname(uint8_t *fatname, uint8_t *fcbname)
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
void fcb_setdrivename(char *arg, uint8_t *drive, uint8_t *filename)
{
    if (arg[1] == ':') {
        if (arg[0] >= 'a' && arg[0] <= 'p')
            *drive = arg[0] - 'a' + 1;
        else if (arg[0] >= 'A' && arg[0] <= 'P')
            *drive = arg[0] - 'A' + 1;
        else
            *drive = 0;
        fcb_setname(arg+2, filename);
    } else {
        fcb_setname(arg, filename);
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
 * Dump the contents of an FCB
 */
#ifdef BDOS_DEBUG
uint8_t fcb_dump(fcb_t *f)
{
    printf_P(PSTR("DR FILENAME EXT   DR FILENAME EXT   CR EX S2   R0 R1 R2   RC S1   &FCB &FIL\n"));
    for (uint8_t j = 0; j < 2; j++) {
        uint8_t *fn;
        uint8_t dr;
        if (j == 0) {
            fn = f->fn;
            dr = f->dr;
        } else {
            fn = f->fn2;
            dr = f->dr2;
        }
        printf_P(PSTR("%02x "), dr);
        for (uint8_t i = 0; i < 11; i++) {
            if (i == 8)
                putchar(' ');
            if (0x20 <= (fn[i] & 0x7f) && (fn[i] & 0x7f) <= 0x7e)
                putchar(fn[i] & 0x7f);
            else
                putchar(' ');
        }
        printf_P(PSTR("   "));
    }
    printf_P(PSTR("%02x "), f->cr);
    if (f->ex == '?')
        printf_P(PSTR("?? "));
    else
        printf_P(PSTR("%02x "), f->ex);
    printf_P(PSTR("%02x   "), f->s2);
    printf_P(PSTR("%02x %02x %02x   "), f->r0, f->r1, f->r2);    
    printf_P(PSTR("%02x %02x   "), f->rc, f->s1);
    printf_P(PSTR("%04x %04x\n"), f->fcbaddr, f->filaddr);
}
#endif

/**
 * Translate FatFS error to BDOS error and log error if appropriate
 */
uint8_t bdos_error(FRESULT fr)
{
    if (fr == FR_OK) {
        return 0;
    } else {
        printf_P(PSTR("BDOS error: %S\n"), strlookup(fr_text, fr));
        return BDOS_ERROR;
    }
}

/**
 * Search for a files matching name in an FCB
 */
uint8_t bdos_search(uint16_t fcbaddr, uint16_t dmaaddr, uint8_t mode, uint8_t *fatfn)
{
    static uint8_t mask[12];
    static uint8_t searchex;
    static uint32_t bytesleft;
    static fcb_t dirfcb;
    static DIR dp;
    FRESULT fr;
    FILINFO fno;
    uint8_t buf[RECSIZ];

    // On first search, save mask and extent, then reopen directory
    if (mode == BDOS_SFIRST) {
        memcpy(mask, reqfcb.fn, 11);
        mask[11] = 0;
        searchex = reqfcb.ex;
        f_closedir(&dp);
        if (fr = f_opendir(&dp, ".") != FR_OK)
            return bdos_error(fr);
        memset(&dirfcb, 0, sizeof(fcb_t));
        bytesleft = 0;
    } 
    
    // Get the next file if no more extents for current file
    if (bytesleft == 0) {    
        do {
            if (fr = f_readdir(&dp, &fno) != FR_OK)
                return bdos_error(fr);
            if (fno.fname[0] == 0)  // indicate end of directory
                return 0xff;
            if (fno.fattrib & AM_DIR)   // skip subdirectories
                continue;
            fcb_setname(fno.fname, dirfcb.fn);
        } while (!fcb_match(mask, dirfcb.fn));  // check filename match

        if (fatfn != NULL)
            strncpy(fatfn, fno.fname, 13);
        
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

    // Check if more extents to return, and decrement bytes left by on extent
    if (bytesleft > EXTSIZ && searchex == '?')
        bytesleft -= EXTSIZ;
    else
        bytesleft = 0;

#ifdef BDOS_DEBUG
    printf_P(PSTR("dir:\n"));
    fcb_dump(&dirfcb);
#endif

    // Pad extra space with empty directory entries
    memcpy(buf, &dirfcb, 32);
    memset(buf+32, 0xe5, RECSIZ-32);
    // Save directory entry to DMA buffer
    mem_write_bare(dmaaddr, buf, RECSIZ);
    return 0;
}

/**
 * Read or write the file associated with the FCB
 */
uint8_t bdos_readwrite(uint16_t fcbaddr, uint16_t dmaaddr, uint8_t mode)
{
    uint32_t offset;
    uint8_t buf[RECSIZ];
    FIL *fp;
    FRESULT fr;
    UINT br;

    // Try to get an active FP for this FCB
    if ((fp = fcb_getfp(FP_FIND, fcbaddr)) == NULL)
        return bdos_error(FR_INVALID_OBJECT);
    // Calculate offset for 
    if (mode == BDOS_READ || mode == BDOS_WRITE) {
        offset = fcb_seqoffset(&reqfcb);
    } else {
        offset = fcb_randoffset(&reqfcb); 
    }
    // Seek to calculated offset if it is not equal to current offset
    if (offset != f_tell(fp))
        if ((fr = f_lseek(fp, offset)) != FR_OK)
            return bdos_error(fr);
    // Do the read or write operation
    if (dma_command == BDOS_READRAND || dma_command == BDOS_READ) {
        if ((fr = f_read(fp, buf, RECSIZ, &br)) != FR_OK)
            return bdos_error(fr);
        // Return EOF if reading past end of file
        if (br == 0)
            return 1;
        // pad incomplete record with EOF
        memset(buf+br, 0x1a, RECSIZ-br); 
        mem_write_bare(params.dmaaddr, buf, RECSIZ);
    } else {
        mem_read_bare(params.dmaaddr, buf, RECSIZ);
        if ((fr = f_write(fp, buf, RECSIZ, &br)) != FR_OK)
            return bdos_error(fr);
    }
    // Automatically increment record if in sequential mode
    if (dma_command == BDOS_READ || dma_command == BDOS_WRITE)
        offset += RECSIZ;
    fcb_setseq(&reqfcb, offset);
#ifdef BDOS_DEBUG
    printf_P(PSTR("after:\n"));
    fcb_dump(&reqfcb);
#endif
    mem_write_bare(params.fcbaddr, &reqfcb, sizeof(fcb_t));
    // Indicate success
    return 0;
}

/**
 * Update random fields in FCB from sequential fields
 */
uint8_t bdos_randrec(uint16_t fcbaddr)
{
    fcb_setrand(&reqfcb, fcb_seqoffset(&reqfcb));
    mem_write_bare(fcbaddr, &reqfcb, sizeof(fcb_t));
    return 0;
}

/**
 * Close file associated with FCB
 */
uint8_t bdos_close(uint16_t fcbaddr)
{
    FIL *fp;
    if (fp = fcb_getfp(FP_REMOVE, fcbaddr))
        return bdos_error(f_close(fp));
}


/**
 * Open file named in FCB
 */
uint8_t bdos_open(uint16_t fcbaddr, uint8_t mode)
{
    FRESULT fr;
    FIL *fp;
    uint8_t fatfn[13];
    bdos_close(fcbaddr);
    if (mode == BDOS_OPEN) {
        mode = FA_READ | FA_WRITE;
        reqfcb.s2 = 0;
    } else  {
        mode = FA_READ | FA_WRITE | FA_CREATE_NEW;
        fcb_setseq(&reqfcb, 0);
    }
    if ((fp = fcb_getfp(FP_NEW, fcbaddr)) == NULL)
        return bdos_error(FR_TOO_MANY_OPEN_FILES);
    fcb_getname(reqfcb.fn, fatfn);
    if ((fr = f_open(fp, fatfn, mode)) != FR_OK)
        return bdos_error(fr);
    mem_write_bare(fcbaddr, &reqfcb, 33);
    return 0;    
}

/**
 * Delete file named in FCB
 */
uint8_t bdos_delete(uint8_t fcbaddr)
{
    FRESULT fr = FR_OK;
    uint8_t fatfn[13];
    // Search for files matching wildcards
    uint8_t ret = bdos_search(fcbaddr, params.dmaaddr, BDOS_SFIRST, fatfn);
    while (!ret) {
        if (fr = f_unlink(fatfn) != FR_OK)
            return bdos_error(fr);
        ret = bdos_search(fcbaddr, params.dmaaddr, BDOS_SNEXT, fatfn);
    }
    return bdos_error(fr);
}

/**
 * Rename file named in fCB
 */
uint8_t bdos_rename(uint16_t fcbaddr)
{
    uint8_t fatfn[13], fatfn2[13];
    fcb_getname(reqfcb.fn, fatfn);
    fcb_getname(reqfcb.fn2, fatfn2);
    return bdos_error(f_rename(fatfn, fatfn2));
}

/**
 * Emulate a BDOS curdrv function using DMA
 */
void bdos_dma_execute()
{
    // Get DMA parameters
    mem_read_bare(dma_mailbox, &params, sizeof(bdos_mailbox_t));
#ifdef BDOS_DEBUG
    printf_P(PSTR("\nBDOS %d %S   fcb %04xh   dma %04xh\n"), dma_command, strlookup(bdos_names, dma_command), params.fcbaddr, params.dmaaddr);
#endif

    // Get the specified FCB
    if (dma_command != BDOS_SNEXT) {
        mem_read_bare(params.fcbaddr, &reqfcb, sizeof(fcb_t));
#ifdef BDOS_DEBUG
        printf_P(PSTR("fcb:\n"));
        fcb_dump(&reqfcb);
#endif
    }

    switch (dma_command) {
        case BDOS_OPEN:
        case BDOS_MAKE:
            params.ret = bdos_open(params.fcbaddr, dma_command);
            break;
        case BDOS_CLOSE:
            params.ret = bdos_close(params.fcbaddr);
            break;
        case BDOS_SFIRST:
        case BDOS_SNEXT:
            params.ret = bdos_search(params.fcbaddr, params.dmaaddr, dma_command, NULL);
            break;
        case BDOS_READ:
        case BDOS_WRITE:
        case BDOS_READRAND:
        case BDOS_WRITERAND:
        case BDOS_WRITEZF:
            params.ret = bdos_readwrite(params.fcbaddr, params.dmaaddr, dma_command);
            break;
        case BDOS_DELETE:
            params.ret = bdos_delete(params.fcbaddr);
            break;
        case BDOS_RENAME:
            params.ret = bdos_rename(params.fcbaddr);
            break;
        case BDOS_ATTRIB:
        case BDOS_SIZE:
        case BDOS_RANDREC:
            params.ret = bdos_randrec(params.fcbaddr);
            break;
        default:
            params.ret = bdos_error(FR_INVALID_PARAMETER);
            break;
    }

    // Write back return values
    mem_write_bare(dma_mailbox, &params, sizeof(bdos_mailbox_t));
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
        fcb_setdrivename(argv[1], &deffcb.dr, deffcb.fn);
    if (argc >= 3)
        fcb_setdrivename(argv[2], &deffcb.dr2, deffcb.fn2);
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
    dma_status = DMA_MAILBOX_UNSET;
    dma_mailbox = 0;
    dma_function = NULL;
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