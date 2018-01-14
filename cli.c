#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "altmon.h"
#include "bus.h"
#include "memory.h"
#include "z80.h"
#include "cli.h"
#include "ff.h"

FATFS fs;

void f_print_error(FRESULT fr) {
    switch(fr) {
        case FR_OK:
            break;
        case FR_DISK_ERR:
            printf_P(PSTR("drive error"));
            break;
        case FR_INT_ERR:
            printf_P(PSTR("internal error"));
            break;
        case FR_NOT_READY:
            printf_P(PSTR("drive not ready"));
            break;
        case FR_NO_FILE:
            printf_P(PSTR("file not found"));
            break;
        case FR_NO_PATH:
            printf_P(PSTR("path not found"));
            break;
        case FR_INVALID_NAME:
            printf_P(PSTR("invalid path name"));
            break;
        case FR_DENIED:
            printf_P(PSTR("access denied"));
            break;
        case FR_EXIST:
            printf_P(PSTR("file already exists"));
            break;
        case FR_INVALID_OBJECT:
            printf_P(PSTR("invalid object"));
            break;
        case FR_WRITE_PROTECTED:
            printf_P(PSTR("write protected"));
            break;
        case FR_INVALID_DRIVE:
            printf_P(PSTR("invalid drive number"));
            break;
        case FR_NOT_ENABLED:
            printf_P(PSTR("drive not mounted"));
            break;
        case FR_NO_FILESYSTEM:
            printf_P(PSTR("invalid filesystem"));
            break;
        case FR_MKFS_ABORTED:
            printf_P(PSTR("mkfs aborted"));
            break;
        case FR_TIMEOUT:
            printf_P(PSTR("timeout"));
            break;
        case FR_LOCKED:
            printf_P(PSTR("locked"));
            break;
        case FR_NOT_ENOUGH_CORE:
            printf_P(PSTR("out of memory"));
            break;
        case FR_TOO_MANY_OPEN_FILES:
            printf_P(PSTR("too many open files"));
            break;
        case FR_INVALID_PARAMETER:
            printf_P(PSTR("invalid parameter"));
            break;
        default:
            printf_P(PSTR("unknown error"));
            break;
    }
}

void cli_loadhex(int argc, char *argv[])
{
    FIL fp;
    FRESULT fr;
    ihex_res res;
    char buf[524];
    char *filename = NULL;
    uint16_t total = 0;
    uint16_t line = 0;
    if (argc == 2) {
        filename = argv[1];
        printf_P(PSTR("loading from %s\n"), filename);
        if ((fr = f_open(&fp, filename, FA_READ)) != FR_OK) {
            printf_P(PSTR("error opening file: "));
            f_print_error(fr);
            putchar('\n');
            return;
        }        
    } else {
        printf_P(PSTR("loading from console; enter blank line to cancel\n"));
    }
    for (;;) {
        if (filename != NULL) {
            if (f_gets(buf, 524, &fp) == NULL) {
                if (f_error(&fp))
                    printf_P(PSTR("error: unable to read file\n"));
                else if (f_eof(&fp))
                    printf_P(PSTR("error: unexpected eof\n"));
                break;
            }
        } else {
            gets(buf);
            if (strlen(buf) == 0)
                break;
        }
        line++;
        res = write_ihex_rec(buf);
        if (res.rc == IHEX_OK && res.type == IHEX_DATA && res.count > 0) {
            printf_P(PSTR("loaded 0x%02X bytes to 0x%04X\n"), res.count, res.addr);
            total += res.count;
        }
        else if (res.rc == IHEX_OK && res.count == 0) {
            printf_P(PSTR("loaded 0x%04X bytes total\n"), total);
            break;
        } else if (res.rc == IHEX_FORMAT)
            printf_P(PSTR("error: invalid record format on line %d\n"), line);
        else if (res.rc == IHEX_COUNT)
            printf_P(PSTR("error: insufficient bytes on line %d\n"), line);
        else if (res.rc == IHEX_CKSUM)
            printf_P(PSTR("error: checksum mismatch on line %d\n"), line);
        else if (res.rc == IHEX_RECTYPE)
            printf_P(PSTR("error: unsupported record type %02XH on line %d\n"), res.type, line);
    }
    if (filename != NULL)
        if ((fr = f_close(&fp)) != FR_OK) {
            printf_P(PSTR("error closing file: "));
            f_print_error(fr);
            putchar('\n');
            return;
        }
}

void cli_savehex(int argc, char *argv[])
{
    FRESULT fr;
    FIL fp;

    uint16_t addr;
    uint16_t length;
    uint8_t count;
    char *filename = NULL;
    char * record;
    uint16_t i;
    if (argc < 3) {
        printf_P(PSTR("usage: savehex <start> <length> <file>\n"));
        return;
    }
    addr = strtol(argv[1], NULL, 16) & 0xffff;
    length = strtol(argv[2], NULL, 16) & 0xffff;
    if (argc == 4) {
        filename = argv[3];
        if ((fr = f_open(&fp, filename, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
            printf_P(PSTR("error opening file: "));
            f_print_error(fr);
            putchar('\n');
            return;
        }        
    }
    for (;;) {
        if (length > 16)
            count = 16;
        else
            count = length;

        printf("saving 0x%02X bytes from 0x%04X\n", count, addr);
        record = read_ihex_rec(addr, count);
        length -= count;
        addr += count;
        if (filename != NULL) {
            if (f_printf(&fp, "%s\n", record) == EOF) {
                printf_P(PSTR("error writing file\n"));
                break;
            }
            if (length == 0) {
                if (f_printf(&fp, "%s\n", read_ihex_rec(0,0)) == EOF) {
                    printf_P(PSTR("error writing file\n"));
                }
                break;
            }
        } else {
            puts(record);
            if (length == 0) {
                puts(read_ihex_rec(0,0));
                break;
            }
        }
    }
    if (filename != NULL) {
        if ((fr = f_close(&fp)) != FR_OK) {
            printf_P(PSTR("error closing file: "));
            f_print_error(fr);
            putchar('\n');
            return;
        }
    }
}

void cli_dump(int argc, char *argv[])
{
    uint16_t addr;
    uint16_t length;
    if (argc != 3) {
        printf_P(PSTR("usage: dump <start> <length>\n"));
        return;
    }
    addr = strtol(argv[1], NULL, 16) & 0xffff;
    length = strtol(argv[2], NULL, 16) & 0xffff;
    dump_mem(addr, length);
}

void cli_run(int argc, char *argv[])
{
    uint16_t addr;
    if (argc != 2) {
        printf_P(PSTR("usage: run <address>\n"));
        return;
    }
    addr = strtol(argv[1], NULL, 16) & 0xffff;
    z80_run(addr);
}

void cli_bank(int argc, char *argv[])
{
    uint8_t bank;
    if (argc != 2) {
        printf_P(PSTR("usage: bank <0-7>\n"));
    }
    bank = strtol(argv[1], NULL, 10) & 0x7;
    SET_BANK(bank);
}

void cli_bus(int argc, char *argv[])
{
    puts(bus_status());
}

void cli_altmon(int argc, char *argv[])
{
    write_mem_P(0xf800, altmon_bin, altmon_bin_len);
    z80_run(0xf800);
}

void cli_dir(int argc, char *argv[])
{
 	FRESULT fr;
    FILINFO Finfo;
    DIR Dir;
	long p1;
	UINT s1, s2, cnt;
    FATFS *fs;

    if (argc == 1)
        argv[1] = "/";

    fr = f_opendir(&Dir, argv[1]);
    if (fr) {
        printf_P(PSTR("error reading directory: "));
        f_print_error(fr);
        putchar('\n');
        return;
    }
    p1 = s1 = s2 = 0;
    for(;;) {
        fr = f_readdir(&Dir, &Finfo);
        if ((fr != FR_OK) || !Finfo.fname[0]) 
            break;
        if (Finfo.fattrib & AM_DIR) {
            s2++;
        } else {
            s1++; p1 += Finfo.fsize;
        }
        printf_P(PSTR("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n"), 
                    (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                    (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                    (Finfo.fattrib & AM_HID) ? 'H' : '-',
                    (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                    (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                    (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                    (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
                    (DWORD)Finfo.fsize, 
                    Finfo.fname);
    }
    if (fr == FR_OK) {
        printf_P(PSTR("%4u File(s),%10lu bytes total\n%4u Dir(s)"), s1, p1, s2);
        if (f_getfree(argv[1], (DWORD*)&p1, &fs) == FR_OK) {
            printf_P(PSTR(", %10luKiB free\n"), p1 * fs->csize / 2);
        }
    }
    if (fr) {
        printf_P(PSTR("error reading directory: "));
        f_print_error(fr);
        putchar('\n');
    };
}

void cli_help(int argc, char *argv[]);

typedef struct _cli_entry {
    char *name;
    char *desc;
    void (*func)(int, char*[]);
} cli_entry;

cli_entry cli_cmds[] = {
    {"altmon", "run altmon 8080 monitor", &cli_altmon},
    {"bus", "display current bus status", &cli_bus},
    {"bank", "select active 64K bank", &cli_bank},
    {"dir", "shows directory listing", &cli_dir},
    {"dump", "dump memory in hex and ascii", &cli_dump},
    {"help", "list available commands", &cli_help},
    {"loadhex", "load intel hex file to memory", &cli_loadhex},
    {"run", "execute code at address", &cli_run},
    {"savehex", "save intel hex file from memory", &cli_savehex}
};

#define NUM_CMDS (sizeof(cli_cmds)/sizeof(cli_entry))

void cli_help(int argc, char *argv[])
{
    int i;
    printf_P(PSTR("available commands:\n"));
    for (i = 0; i < NUM_CMDS; i++) {
        printf_P(PSTR("%s\t%s\n"), cli_cmds[i].name, cli_cmds[i].desc);
    }
}

#define WHITESPACE " \t\r\n"
#define MAXBUF 80
#define MAXARGS 8

void cli_loop(void) {
    char buf[MAXBUF];
    char *cmd;
    char *argv[MAXARGS];
    int argc;
    int i;
    FRESULT fr;

    disk_initialize(0);
    if ((fr = f_mount(&fs, "", 1)) != FR_OK) {
        printf_P(PSTR("error mounting drive: "));
        f_print_error(fr);
        putchar('\n');
    }

    printf_P(PSTR("type help to list available commands\n"));
    for (;;) {
        printf_P(PSTR("z80ctrl>"));
        if (fgets(buf, sizeof buf - 1, stdin) != NULL) {
            if ((argv[0] = strtok(buf, WHITESPACE)) == NULL)
                continue;            
            for (argc = 1; argc < MAXARGS; argc++) {
                if ((argv[argc] = strtok(NULL, WHITESPACE)) == NULL)
                    break;
            }
            for (i = 0; i < NUM_CMDS; i++) {
                if (strcmp(argv[0], cli_cmds[i].name) == 0) {
                    cli_cmds[i].func(argc, argv);
                    break;
                }
            }
            if (i == NUM_CMDS) {
                printf_P(PSTR("unknown command: %s. type help for list.\n"), argv[0]);
            }
        }
    }
}