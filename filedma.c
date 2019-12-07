#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "filedma.h"
#include "ff.h"
#include "bus.h"
#include "iorq.h"
#include "util.h"

typedef enum {
    F_OPEN,
    F_CLOSE,
    F_READ,
    F_WRITE,
    F_LSEEK,
    F_TRUNCATE,
    F_SYNC,
    F_FORWARD,
    F_EXPAND,
    F_GETS,
    F_PUTC,
    F_PUTS,
    F_PRINTF,
    F_TELL,
    F_EOF,
    F_SIZE,
    F_ERROR,

    F_OPENDIR,
    F_CLOSEDIR,
    F_READDIR,
    F_FINDFIRST,
    F_FINDNEXT,

    F_STAT,
    F_UNLINK,
    F_RENAME,
    F_CHMOD,
    F_UTIME,
    F_MKDIR,
    F_CHDIR,
    F_CHDRIVE,
    F_GETCWD,

    F_MOUNT,
    F_MKFS,
    F_FDISK,
    F_GETFREE,
    F_GETLABEL,
    F_SETLABEL,
    F_SETCP,

    F_DISK_STATUS,
    F_DISK_INITIALIZE,
    F_DISK_READ,
    F_DISK_WRITE,
    F_DISK_IOCTL,

    F_GET_FATTIME,

    F_GET_CLIBUF = 64
} dma_command_t;

static uint16_t dma_mailbox = 0;
static uint8_t dma_command = 0;
static dma_status_t dma_status = DMA_MAILBOX_UNSET;

typedef struct {
    uint16_t fpaddr;
    uint16_t inaddr;
    uint16_t outaddr;
    uint16_t maxlen;
    uint16_t retlen;
    uint32_t ofs;
    uint8_t mode;
    uint8_t mask;
    uint8_t fr;
} dma_mailbox_t;

/**
 * Reset the DMA parameter address
 */
uint8_t file_dma_reset()
{
    dma_status = DMA_MAILBOX_UNSET;
    dma_mailbox = 0;
    dma_function = NULL;
    return 0xff;
}

/*
 * Execute queued DMA command
 */
void file_dma_execute()
{
    dma_mailbox_t params;
    FILINFO fno;
    union {
        FIL fp;
        DIR dp;
    } obj;
    const size_t objsize = sizeof(FIL) >= sizeof(DIR) ? sizeof(FIL) : sizeof(DIR);
    uint8_t buf[256];
    uint8_t *bufp;
    uint16_t buflen;

    mem_read_bare(dma_mailbox, &params, sizeof(dma_mailbox_t));
    mem_read_bare(params.fpaddr, &obj.dp, objsize);
    buflen = params.maxlen < 256 ? params.maxlen : 256;

    //printf("executing dma: cmd %02x, inaddr %04x, outaddr %04x, length %04x\n", params.cmd, params.inaddr, params.outaddr, params.maxlen);

    switch (dma_command) {
        case F_OPEN:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_open(&obj.fp, buf, params.mode);
            break;
        case F_CLOSE:
            params.fr = f_close(&obj.fp);
            break;
        case F_READ:
            params.fr = file_to_mem(&obj.fp, params.outaddr, params.maxlen, &params.retlen);
            break;
        case F_WRITE:
            params.fr = mem_to_file(&obj.fp, params.inaddr, params.maxlen, &params.retlen);
            break;
        case F_LSEEK:
            params.fr = f_lseek(&obj.fp, params.ofs);
            break;
        case F_TRUNCATE:
            params.fr = f_truncate(&obj.fp);
            break;
        case F_SYNC:
            params.fr = f_sync(&obj.fp);
            break;
#if F_USE_EXPAND && !F_FS_READONLY
        case F_EXPAND:
            params.fr = f_expand(&obj.fp, params.ofs, params.mode);
            break;
#endif
        case F_TELL:
            params.ofs = f_tell(&obj.fp);
            params.fr = FR_OK;
            break;
        case F_EOF:
            params.fr = f_eof(&obj.fp);
            break;
        case F_SIZE:
            params.ofs = f_size(&obj.fp);
            params.fr = FR_OK;
            break;
        case F_ERROR:
            params.fr = f_error(&obj.fp);
            break;
        case F_OPENDIR:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_opendir(&obj.dp, buf);
            break;
        case F_CLOSEDIR:
            params.fr = f_closedir(&obj.dp);
            break;
        case F_READDIR:
            if ((params.fr = f_readdir(&obj.dp, &fno)) == FR_OK)
                mem_write_bare(params.outaddr, &fno, sizeof fno);
            break;
        case F_FINDFIRST:
            mem_read_bare(params.inaddr, buf, buflen);
            bufp = strchr(buf, 0) + 1;
            if ((params.fr = f_findfirst(&obj.dp, &fno, buf, bufp)) == FR_OK)
                mem_write_bare(params.outaddr, &fno, sizeof fno);
            break;
        case F_FINDNEXT:
            if ((params.fr = f_findnext(&obj.dp, &fno)) == FR_OK)
                mem_write_bare(params.outaddr, &fno, sizeof fno);
            break;
        case F_STAT:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_stat(buf, &fno);
            break;
        case F_UNLINK:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_unlink(buf);
            break;
        case F_RENAME:
            mem_read_bare(params.inaddr, buf, buflen);
            bufp = strchr(buf, 0) + 1;
            params.fr = f_rename(buf, bufp);
            break;
#if F_USE_CHMOD && !F_FS_READONLY
        case F_CHMOD:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_chmod(buf, params.mode, params.mask);
            break;
        case F_UTIME:
            mem_read_bare(params.inaddr, buf, buflen);
            if ((params.fr = f_utime(buf, &fno)) == FR_OK)
                mem_write_bare(params.outaddr, &fno, sizeof fno);
            break;
#endif
        case F_MKDIR:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_mkdir(buf);
            break;
        case F_CHDIR:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_chdir(buf);
            break;
#if F_VOLUMES >= 2
        case F_CHDRIVE:
            mem_read_bare(params.inaddr, buf, buflen);
            params.fr = f_chdrive(buf);
            break;
#endif
        case F_GETCWD:
            if ((params.fr = f_getcwd(buf, buflen)) == FR_OK)
                mem_write_bare(params.outaddr, buf, buflen);
            break;
        case F_GET_CLIBUF:
            mem_write_bare(params.outaddr, clibuf, buflen);
            params.fr = FR_OK;
            break;
        default:
            params.fr = FR_INVALID_PARAMETER;
            break;
    }
    //printf("completed with status %02x\n", params.fr);
    mem_write_bare(dma_mailbox, &params, sizeof(dma_mailbox_t));
    mem_write_bare(params.fpaddr, &obj.dp, objsize);
}

/**
 * Queue a DMA command for execution
 */
void file_dma_command(uint8_t data) 
{
    switch (dma_status) {
        case DMA_MAILBOX_UNSET:
            dma_mailbox = data;
            dma_status = DMA_MAILBOX_HALFSET;
            break;
        case DMA_MAILBOX_HALFSET:
            dma_mailbox |= (data << 8);
            dma_status = DMA_MAILBOX_SET;
            //printf("mailbox base set to %04x\n", dma_mailbox);
            break;
        case DMA_MAILBOX_SET:
            dma_command = data;
            dma_function = file_dma_execute;
            //printf("received dma request on mailbox %02x\n", dma_command);
            break;
        default:
            dma_function = NULL;
    }
}