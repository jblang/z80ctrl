#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bdosemu.h"
#include "iorq.h"
#include "util.h"
#include "uart.h"
#include "ff.h"

typedef struct {
    uint8_t drive;
    uint8_t filename[12];    
    uint8_t extent;
    uint8_t s1, s2;
    uint8_t recordcnt;
    uint8_t data[16];
    uint8_t currecord;
    uint16_t random;
    uint8_t randomovf;
} fcb_t;

typedef struct {
    uint16_t fcb;
    uint16_t ret;
    uint8_t disk;
    uint16_t dma;
    uint16_t scratch;
} bdos_mailbox_t;

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

static uint16_t dma_mailbox = 0;
static uint8_t dma_command = 0;
static dma_status_t dma_status = DMA_MAILBOX_UNSET;

#define NOFILES 16
#define NODRIVES 4

#define NEW 0
#define FIND 1
#define REMOVE 2

/*
 * Find FatFS file object associated with FCB address
 */
uint8_t getfp(uint8_t mode, uint16_t base, uint16_t fcbaddr, FIL *fp)
{
    static uint16_t lastfcb = 0;
    static uint16_t lastfp = 0;
    uint16_t findfcb;
    uint16_t fcbaddrs[NOFILES];

    // Find empty slot or existing FCB depending on mode
    if (mode == NEW) {
        //printf_P(PSTR("new fcb requested; finding empty slot\n"));
        findfcb = 0;
    } else {
        //printf_P(PSTR("finding existing fp for fcb at %04xh\n"), fcbaddr);
        findfcb = fcbaddr;
    }

    // Nothing to do if looking for same FCB as last time
    if (mode == FIND && findfcb == lastfcb) {
        //printf_P(PSTR("same fcb as last time; skip search\n"));
        return 1;
    }

    // If last fp was active, save it back to external ram
    if (lastfp != 0) {
        //printf_P(PSTR("writing previous fp back to memory at %04xh\n"), lastfp);
        mem_write_bare(lastfp, fp, sizeof(FIL));
    }

    // Get list of active FCB addresses
    mem_read_bare(base, fcbaddrs, NOFILES*2);
    
    // Search for matching FCB address
    for (uint8_t i = 0; i < NOFILES; i++) {
        if (findfcb == fcbaddrs[i]) {
            // Cache address of matching file pointer
            lastfp = base + NOFILES * 2 + sizeof(FIL) * i;

            if (mode == NEW) {
                // New FCB, claim address slot, no need to read FP
                //printf_P(PSTR("slot %d is empty; claiming for fcb at %04xh\n"), i, fcbaddr);
                mem_write_bare(base + i * 2, &fcbaddr, 2);
            } else {
                // Existing FCB, read in matching FP from external ram
                //printf_P(PSTR("slot %d matches requested fcb %04xh; reading corresponding fp at %04xh\n"), i, fcbaddr, lastfp);
                mem_read_bare(lastfp, fp, sizeof(FIL));
            }

            if (mode == REMOVE) {
                // Removing FCB; set slot to 0
                lastfp = 0;
                fcbaddr = 0;
                //printf_P(PSTR("fcb removal requested; clearing slot %d\n"), i);
                mem_write_bare(base + i * 2, &fcbaddr, 2);
            }

            // Cache last FCB address
            lastfcb = fcbaddr;
            return 1; // found
        }
    }
    if (mode == NEW) {
        //printf_P(PSTR("no empty fcb slots available\n"));
    } else {
        //printf_P(PSTR("fcb %04xh not found\n"), findfcb);
    }
    return 0; // not found
}

DIR dir;

/*
 * Convert CP/M FCB filename to FAT format
 */
void fcb2fat(char *fcbname, char *fatname)
{
    uint8_t i = 0, j = 0;
    while (i < 8) {
        if (fcbname[i] != ' ')
            fatname[j++] = fcbname[i] & 0x7f;
        i++;
    }
    fatname[j++] = '.';
    while (i < 11) {
        if (fcbname[i] != ' ')
            fatname[j++] = fcbname[i] & 0x7f;
        i++;
    }
    fatname[j] = 0;
    //printf_P(PSTR("fcb2fat: %s -> %s\n"), fcbname, fatname);
}

/*
 * Convert FAT filename to CP/M FCB format
 */
void fat2fcb(char *fatname, char *fcbname)
{
    uint8_t i = 0, j = 0;
    while (i < 8)
        if (fatname[j] == '.' || fatname[j] == 0)
            fcbname[i++] = ' ';
        else if (fatname[j] == '*')
            fcbname[i++] = '?';
        else
            fcbname[i++] = fatname[j++] & 0x7f;
    while (fatname[j] == '*' || fatname[j] == '.')
        j++;
    while (i < 11)
        if (fatname[j] == 0)
            fcbname[i++] = ' ';
        else if (fatname[j] == '*')
            fcbname[i++] = '?';
        else
            fcbname[i++] = fatname[j++] & 0x7f;
    //printf_P(PSTR("fat2fcb: %s -> %s\n"), fatname, fcbname);
}

/*
 * Find next file in directory using CP/M wildcard rules
 */
FRESULT findnext(DIR *dp, char *mask, char *fatname)
{
    FILINFO fi;
    FRESULT fr;
    char fcbname[13];
    //printf_P(PSTR("mask filename: %s\n"), mask);
nextfile:
    fr = f_readdir(dp, &fi);
    if (fr != FR_OK)
        return fr;
    //printf_P(PSTR("directory filename: %s\n"), fi.fname);
    if (fi.fname[0] == 0)
        return FR_NO_FILE;
    if (fi.fattrib & AM_DIR)
        goto nextfile;
    fat2fcb(fi.fname, fcbname);
    for (uint8_t i = 0; i < 11; i++) {
        //printf_P(PSTR("%c%c|"), toupper(mask[i]), fcbname[i]);
        if (mask[i] != '?' && toupper(mask[i]) != fcbname[i]) {
            //printf_P(PSTR(" didn't match\n"));
            goto nextfile;
        }
    }
    //printf_P(PSTR(" matched\n"));
    strcpy(fatname, fi.fname);
    return FR_OK;
}

/*
 * Find first file in directory using CP/M wildcard rules
 */
FRESULT findfirst(DIR *dp, char *mask, char *fatname)
{
    f_closedir(dp);
    FRESULT fr = f_opendir(dp, ".");
    if (fr != FR_OK)
        return fr;
    return findnext(dp, mask, fatname);
}

/*
 * Emulate a BDOS disk function using DMA
 */
void bdos_dma_execute()
{
    FRESULT fr;
    static FIL fp;
    static DIR dp;

    bdos_mailbox_t params;
    fcb_t fcb;

    char fatfn[13], fatfn2[13];
    uint8_t buf[128];
    UINT br;

    mem_read_bare(dma_mailbox, &params, sizeof(bdos_mailbox_t));
    mem_read_bare(params.fcb, &fcb, sizeof(fcb_t));

    //printf_P(PSTR("BDOS mailbox %04xh, function %d, fcb %04xh, dma %04xh, scratch %04xh\n"), dma_mailbox, dma_command, params.fcb, params.dma, params.scratch);

    switch (dma_command) {
        case BDOS_OPEN:
            //printf_P(PSTR("open file %04xh\n"), params.fcb);
            if (getfp(REMOVE, params.scratch, params.fcb, &fp))
                f_close(&fp);
            fr = findfirst(&dp, fcb.filename, fatfn);
            if (fr == FR_OK) {
                //printf_P(PSTR("open filename: %s\n"), fatfn);
                if (getfp(NEW, params.scratch, params.fcb, &fp)) {
                    fr = f_open(&fp, fatfn, FA_READ | FA_WRITE);
                    fcb.extent = 0;
                    fcb.currecord = 0;
                    mem_write_bare(params.fcb, &fcb, sizeof(fcb));
                } else {
                    fr = FR_TOO_MANY_OPEN_FILES;
                }
            }
            break;
        case BDOS_MAKE:
            //printf_P(PSTR("make file %04xh\n"), params.fcb);
            if (getfp(REMOVE, params.scratch, params.fcb, &fp))
                f_close(&fp);
            if (getfp(NEW, params.scratch, params.fcb, &fp)) {
                //printf_P(PSTR("fcb filename %s\n"), fcb.filename);
                fcb2fat(fcb.filename, fatfn);
                //printf_P(PSTR("fat filename: %s\n"), fatfn);
                fr = f_open(&fp, fatfn, FA_READ | FA_WRITE | FA_CREATE_NEW);
            } else {
                fr = FR_TOO_MANY_OPEN_FILES;
            }
            break;
        case BDOS_CLOSE:
            //printf_P(PSTR("close file %04xh\n"), params.fcb);
            if (getfp(REMOVE, params.scratch, params.fcb, &fp)) {
                fr = f_close(&fp);
            } else {
                fr = FR_INVALID_OBJECT;
            }
            break;
        case BDOS_SFIRST:
            //printf_P(PSTR("find first %s\n"), fcb.filename);
            fr = findfirst(&dp, fcb.filename, fatfn);
            if (fr == FR_OK) {
                fat2fcb(fatfn, fcb.filename);
                mem_write_bare(params.dma, &fcb, sizeof(fcb_t));
            }
            break;
        case BDOS_SNEXT:
            //printf_P(PSTR("find next %s\n"), fcb.filename);
            fr = findnext(&dp, fcb.filename, fatfn);
            if (fr == FR_OK) {
                fat2fcb(fatfn, fcb.filename);
                mem_write_bare(params.dma, &fcb, sizeof(fcb_t));
            }
            break;
        case BDOS_READ:
            //printf_P(PSTR("read file %04xh\n"), params.fcb);
            if (getfp(FIND, params.scratch, params.fcb, &fp)) {
                fr = f_lseek(&fp, (fcb.extent * 256 + fcb.currecord) * 128);
                if (fr == FR_OK) {
                    fr = f_read(&fp, buf, 128, &br);
                    if (fr == FR_OK) {
                        for (uint8_t i = br; i < 128; i++)
                            buf[i] = 0x1a;
                        mem_write_bare(params.dma, buf, br);
                        fcb.currecord++;
                        if (fcb.currecord == 0)
                            fcb.extent++;
                        mem_write_bare(params.fcb, &fcb, sizeof(fcb_t));
                    }
                }
            } else {
                fr = FR_INVALID_OBJECT;
            }
            break;
        case BDOS_WRITE:
            //printf_P(PSTR("write file %04xh\n"), params.fcb);
            if (getfp(FIND, params.scratch, params.fcb, &fp)) { 
                fr = f_lseek(&fp, (fcb.extent * 256 + fcb.currecord) * 128);
                if (fr == FR_OK) {
                    mem_read_bare(params.dma, buf, 128);
                    fr = f_write(&fp, buf, 128, &br);
                    if (fr == FR_OK) {
                        fcb.currecord++;
                        if (fcb.currecord == 0)
                            fcb.extent++;
                        mem_write_bare(params.fcb, &fcb, sizeof(fcb_t));
                    }
                }
            } else {
                fr = FR_INVALID_OBJECT;
            }
            break;
        case BDOS_DELETE:
            //printf_P(PSTR("delete file %04xh\n"), params.fcb);
            fr = findfirst(&dp, fcb.filename, fatfn);
            while (fr == FR_OK) {
                //printf_P(PSTR("delete filename: %s\n"), fatfn);
                fr = f_unlink(fatfn);
                if (fr == FR_OK)
                    fr = findnext(&dp, fcb.filename, fatfn);
            }
            break;
        case BDOS_RENAME:
            //printf_P(PSTR("rename file %04xh\n"), params.fcb);
            fcb2fat(fcb.filename, fatfn);
            fcb2fat(fcb.data+1, fatfn2);
            //printf_P(PSTR("rename filename: %s -> %s\n"), fatfn, fatfn2);
            fr = f_rename(fatfn, fatfn2);
            break;
        /*
        case BDOS_ATTRIB:
        case BDOS_READRAND:
        case BDOS_WRITERAND:
        case BDOS_SIZE:
        case BDOS_RANDREC:
        case BDOS_WRITEZF:
        */
        default:
            printf_P(PSTR("Unimplemented BDOS function %d\n"), dma_command);
            fr = FR_INVALID_PARAMETER;
            break;
    }

    if (fr == FR_OK) {
        if (dma_command == BDOS_READ && br == 0)
            params.ret = 1;
        else
            params.ret = 0;
    } else {
        params.ret = 0xff;
        printf_P(PSTR("BDOS function %d error: %S\n"), dma_command, strlookup(fr_text, fr));
    }

    mem_write_bare(dma_mailbox, &params, sizeof(bdos_mailbox_t));
}

/**
 * Initialize default FCB and command tail
 */
void bdos_init(int argc, char *argv[])
{
    char comtail[256];

    fcb_t fcb;
    memset(&fcb, 0, sizeof(fcb_t));
    if (argc >= 2) {
        if (argv[1][1] == ':') {
            if (argv[1][0] >= 'a' && argv[1][0] <= 'p')
                fcb.drive = argv[1][0] - 'a' + 1;
            if (argv[1][0] >= 'A' && argv[1][0] <= 'P')
                fcb.drive = argv[1][0] - 'A' + 1;
            fat2fcb(argv[1]+2, fcb.filename);
        } else {
            fat2fcb(argv[1], fcb.filename);
        }
    }
    if (argc >= 3) {
        if (argv[2][1] == ':') {
            if (argv[2][0] >= 'a' && argv[2][0] <= 'p')
                fcb.data[0] = argv[2][0] - 'a' + 1;
            if (argv[2][0] >= 'A' && argv[2][0] <= 'P')
                fcb.data[0] = argv[2][0] - 'A' + 1;
            fat2fcb(argv[2]+2, fcb.data+1);
        } else {
            fat2fcb(argv[2], fcb.data+1);
        }
    }
    mem_write(0x5c, &fcb, sizeof(fcb_t));
    memset(comtail, ' ', 256);
    char *next = comtail + 1;
    for (int i = 1; i < argc; i++) {
        strcpy(next, argv[i]);
        next += strlen(argv[i]);
        *next++ = ' ';
    }
    comtail[0] = (uint8_t)(next - comtail - 1);
    mem_write(0x80, comtail, 128);
}

/**
 * Reset the DMA parameter address
 */
uint8_t bdos_dma_reset()
{
    dma_status = DMA_MAILBOX_UNSET;
    dma_mailbox = 0;
    dma_function = NULL;
    //printf_P(PSTR("mailbox address reset\n"));
    return 0xff;
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
            //printf_P(PSTR("received lower address %02x\n"), data);
            break;
        case DMA_MAILBOX_HALFSET:
            dma_mailbox |= (data << 8);
            dma_status = DMA_MAILBOX_SET;
            //printf_P(PSTR("recieved upper address %02x; mailbox base set to %04x\n"), data, dma_mailbox);
            break;
        case DMA_MAILBOX_SET:
            dma_command = data;
            dma_function = bdos_dma_execute;
            //printf_P(PSTR("received dma command %02x\n"), dma_command);
            break;
        default:
            dma_function = NULL;
    }
};

