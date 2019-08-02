#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "filedma.h"
#include "ff.h"
#include "bus.h"
#include "iorq.h"

typedef enum {
    DMA_MAILBOX_UNSET = 0,
    DMA_MAILBOX_HALFSET,
    DMA_MAILBOX_SET
} dma_status_t;

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

    F_GET_FATTIME
} dma_command_t;

uint16_t dma_mailbox_base = 0;
uint8_t dma_mailbox_no = 0;
dma_status_t dma_mailbox_status = DMA_MAILBOX_UNSET;

typedef struct {
    uint8_t cmd;
    uint8_t fr;
    uint16_t inaddr;
    uint16_t outaddr;
    uint16_t length;
    uint32_t ofs;
    uint8_t mode;
    uint8_t mask;
    union {
        FIL fp;
        DIR dp;
    };
} dma_mailbox_t;

/**
 * Reset the DMA parameter address
 */
uint8_t file_dma_reset()
{
    dma_mailbox_status = DMA_MAILBOX_UNSET;
    dma_mailbox_base = 0;
    dma_function = NULL;
    return 0xff;
}

/*
 * Execute queued DMA command
 */
void file_dma_execute()
{
    FILINFO fno;
    UINT btr, br, btw, bw;
    dma_mailbox_t data;
    uint8_t buf[256];
    uint8_t buf2[256];

    mem_read_bare(dma_mailbox_base + sizeof(dma_mailbox_t) * dma_mailbox_no, &data, sizeof(dma_mailbox_t));

    //printf("executing dma: cmd %02x, inaddr %04x, outaddr %04x, length %04x\n", data.cmd, data.inaddr, data.outaddr, data.length);

    switch (data.cmd) {
        case F_OPEN:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_open(&data.fp, buf, data.mode);
            break;
        case F_CLOSE:
            data.fr = f_close(&data.fp);
            break;
        case F_READ:
            btr = data.length;
            data.length = 0;
            for (;;) {
                if (btr <= 256) {
                    if ((data.fr = f_read(&data.fp, buf, btr, &br)) != FR_OK)
                        break;
                    mem_write_bare(data.outaddr + data.length, buf, br);
                    data.length += br;
                    break;
                } else {
                    if ((data.fr = f_read(&data.fp, buf, 256, &br)) != FR_OK)
                        break;
                    mem_write_bare(data.outaddr + data.length, buf, br);
                    data.length += br;
                    if (br < 256)
                        break;
                }
                btr -= 256;
            }
            break;
        case F_WRITE:
            btw = data.length;
            data.length = 0;
            for (;;) {
                if (btw <= 256) {
                    mem_read_bare(data.inaddr, buf, btw);
                    if ((data.fr = f_write(&data.fp, buf, btw, &bw)) != FR_OK)
                        break;
                    data.length += bw;
                    break;
                } else {
                    mem_read_bare(data.inaddr + data.length, buf, 256);
                    if ((data.fr = f_write(&data.fp, buf, 256, &bw)) != FR_OK)
                        break;
                    data.length += bw;
                    if (bw < 256)
                        break;
                }
                btw -= 256;
            }
            break;
        case F_LSEEK:
            data.fr = f_lseek(&data.fp, data.ofs);
            break;
        case F_TRUNCATE:
            data.fr = f_truncate(&data.fp);
            break;
        case F_SYNC:
            data.fr = f_sync(&data.fp);
            break;
#if F_USE_EXPAND && !F_FS_READONLY
        case F_EXPAND:
            data.fr = f_expand(&data.fp, data.ofs, data.mode);
            break;
#endif
        case F_TELL:
            data.ofs = f_tell(&data.fp);
            data.fr = FR_OK;
            break;
        case F_EOF:
            data.fr = f_eof(&data.fp);
            break;
        case F_SIZE:
            data.ofs = f_size(&data.fp);
            data.fr = FR_OK;
            break;
        case F_ERROR:
            data.fr = f_error(&data.fp);
            break;
        case F_OPENDIR:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_opendir(&data.dp, buf);
            break;
        case F_CLOSEDIR:
            data.fr = f_closedir(&data.dp);
            break;
        case F_READDIR:
            if ((data.fr = f_readdir(&data.dp, &fno)) == FR_OK)
                mem_write_bare(data.outaddr, &fno, sizeof fno);
            break;
        case F_FINDFIRST:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            mem_read_bare(data.inaddr + (data.length & 0xff), buf2, data.length & 0xff);
            if ((data.fr = f_findfirst(&data.dp, &fno, buf, buf2)) == FR_OK)
                mem_write_bare(data.outaddr, &fno, sizeof fno);
            break;
        case F_FINDNEXT:
            if ((data.fr = f_findnext(&data.dp, &fno)) == FR_OK)
                mem_write_bare(data.outaddr, &fno, sizeof fno);
            break;
        case F_STAT:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_stat(buf, &fno);
            break;
        case F_UNLINK:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_unlink(buf);
            break;
        case F_RENAME:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            mem_read_bare(data.inaddr + (data.length & 0xff), buf2, data.length & 0xff);
            data.fr = f_rename(buf, buf2);
            break;
#if F_USE_CHMOD && !F_FS_READONLY
        case F_CHMOD:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_chmod(buf, data.mode, data.mask);
            break;
        case F_UTIME:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            if ((data.fr = f_utime(buf, &fno)) == FR_OK)
                mem_write_bare(data.outaddr, &fno, sizeof fno);
            break;
#endif
        case F_MKDIR:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_mkdir(buf);
            break;
        case F_CHDIR:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_chdir(buf);
            break;
#if F_VOLUMES >= 2
        case F_CHDRIVE:
            mem_read_bare(data.inaddr, buf, data.length & 0xff);
            data.fr = f_chdrive(buf);
            break;
#endif
        case F_GETCWD:
            if ((data.fr = f_getcwd(buf, data.length & 0xff)) == FR_OK)
                mem_write_bare(data.outaddr, buf, data.length & 0xff);
            break;
        default:
            data.fr = FR_INVALID_PARAMETER;
            break;
    }
    //printf("completed with status %02x\n", data.fr);
    mem_write_bare(dma_mailbox_base + sizeof(dma_mailbox_t) * dma_mailbox_no, &data, sizeof(dma_mailbox_t));
}

/**
 * Queue a DMA command for execution
 */
void file_dma_command(uint8_t data) 
{
    switch (dma_mailbox_status) {
        case DMA_MAILBOX_UNSET:
            dma_mailbox_base = data;
            dma_mailbox_status = DMA_MAILBOX_HALFSET;
            break;
        case DMA_MAILBOX_HALFSET:
            dma_mailbox_base |= (data << 8);
            dma_mailbox_status = DMA_MAILBOX_SET;
            //printf("mailbox base set to %04x\n", dma_mailbox_base);
            break;
        case DMA_MAILBOX_SET:
            dma_mailbox_no = data;
            dma_function = file_dma_execute;
            //printf("received dma request on mailbox %02x\n", dma_mailbox_no);
            break;
        default:
            dma_function = NULL;
    }
}