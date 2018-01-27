#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "altmon.h"
#include "dbl.h"
#include "simhboot.h"
#include "bus.h"
#include "memory.h"
#include "z80.h"
#include "cli.h"
#include "ff.h"
#include "ihex.h"

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
    char ihex[524];
    uint8_t bin[256];
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
            if (f_gets(ihex, 524, &fp) == NULL) {
                if (f_error(&fp))
                    printf_P(PSTR("error: unable to read file\n"));
                else if (f_eof(&fp))
                    printf_P(PSTR("error: unexpected eof\n"));
                break;
            }
        } else {
            gets(ihex);
            if (strlen(ihex) == 0)
                break;
        }
        line++;
        res = ihex_to_bin(ihex, bin);
        if (res.rc == IHEX_OK && res.type == IHEX_DATA && res.count > 0) {
            write_mem(res.addr, bin, res.count);
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

#define BYTESPERLINE 16

void cli_savehex(int argc, char *argv[])
{
    FRESULT fr;
    FIL fp;

    uint32_t start;
    uint16_t end;
    uint8_t count;
    char *filename = NULL;
    char bin[BYTESPERLINE];
    char ihex[BYTESPERLINE*2+12];
    uint16_t i;
    if (argc < 3) {
        printf_P(PSTR("usage: savehex <start> <end> [file]\n"));
        return;
    }
    start = strtol(argv[1], NULL, 16) & 0xffff;
    end = strtol(argv[2], NULL, 16) & 0xffff;
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
        if (end - start + 1 > BYTESPERLINE)
            count = BYTESPERLINE;
        else
            count = end - start + 1;
        read_mem(start, bin, count);
        bin_to_ihex(bin, ihex, start, count);
        if (filename != NULL) {
            if (f_printf(&fp, "%s\n", ihex) == EOF) {
                printf_P(PSTR("error writing file\n"));
                break;
            }
        } else {
            puts(ihex);
        }
        start += count;
        if (start > end) {
            bin_to_ihex(bin, ihex, 0, 0);
            if (filename != NULL) {
                if (f_printf(&fp, "%s\n", ihex) == EOF)
                    printf_P(PSTR("error writing file\n"));
            } else
                puts(ihex);
            break;
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
    uint16_t start, end;
    if (argc < 2) {
        printf_P(PSTR("usage: dump <start> [end]\n"));
        return;
    }
    start = strtol(argv[1], NULL, 16) & 0xffff;
    if (argc < 3)
        end = start + 0xff;
    else
        end = strtol(argv[2], NULL, 16) & 0xffff;
    printf("%04x %04x\n", start, end);
    dump_mem(start, end);
}

void cli_run(int argc, char *argv[])
{
    if (argc >= 2) {
        uint16_t addr = strtol(argv[1], NULL, 16) & 0xffff;
        z80_reset(addr);
    }
    z80_run();
}

void cli_reset(int argc, char *argv[])
{
    uint16_t addr = 0;
    if (argc >= 2) {
        addr = strtol(argv[1], NULL, 16) & 0xffff;
    }
    z80_reset(addr);
}

void cli_debug(int argc, char *argv[])
{
    if (argc >= 2) {
        uint16_t addr = strtol(argv[1], NULL, 16) & 0xffff;
        z80_reset(addr);
    }
    z80_trace(0);
}

void cli_step(int argc, char *argv[])
{
    uint32_t cycles = 1;
    if (argc >= 2)
        cycles = strtol(argv[1], NULL, 16);            
    z80_trace(cycles);
}

void cli_status(int argc, char *argv[])
{
    z80_status();
}

void breakwatch_status(char *name, uint16_t start, uint16_t end)
{
    if (start < end)
        printf_P(PSTR("\t%s: %04x-%04x\n"), name, start, end);
    else if (start == end)
        printf_P(PSTR("\t%s: %04x\n"), name, start);
    else
        printf_P(PSTR("\t%s: disabled\n"), name);
}

void cli_breakwatch(int argc, char *argv[])
{
    uint16_t start = 0;
    uint16_t end = 0xffff;
    if (argc == 1) {
        if (strcmp(argv[0], "break") == 0) {
            printf_P(PSTR("break status:\n"));
            breakwatch_status("memrd", memrd_break_start, memrd_break_end);
            breakwatch_status("memwr", memwr_break_start, memwr_break_end);
            breakwatch_status("iord", iord_break_start, iord_break_end);
            breakwatch_status("iowr", iowr_break_start, iowr_break_end);
            breakwatch_status("opfetch", opfetch_break_start, opfetch_break_end);
        } else {
            printf_P(PSTR("watch status:\n"));
            breakwatch_status("memrd", memrd_watch_start, memrd_watch_end);
            breakwatch_status("memwr", memwr_watch_start, memwr_watch_end);
            breakwatch_status("iord", iord_watch_start, iord_watch_end);
            breakwatch_status("iowr", iowr_watch_start, iowr_watch_end);
            breakwatch_status("opfetch", opfetch_watch_start, opfetch_watch_end);
        }
        printf_P(PSTR("\nusage:\n\t%s <type> [start] [end]\n"), argv[0]);
        printf_P(PSTR("\t%s <type> off to disable type\n"), argv[0]);
        printf_P(PSTR("\t%s off to disable all\n"), argv[0]);
        return;
    }
    if (argc >= 3) {
        if (strcmp(argv[2], "off") == 0) {
            start = 0xffff;
            end = 0;
        } else {
            start = strtol(argv[2], NULL, 16);
            if (argc >= 4)
                end = strtol(argv[3], NULL, 16);
            else
                end = start;
        }
    }
    if (strcmp(argv[1], "memrd") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            memrd_break_start = start;
            memrd_break_end = end;
        } else {
            memrd_watch_start = start;
            memrd_watch_end = end;
        }
    } else if (strcmp(argv[1], "memwr") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            memwr_break_start = start;
            memwr_break_end = end;
        } else {
            memwr_watch_start = start;
            memwr_watch_end = end;
        }
    } else if (strcmp(argv[1], "iord") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            iord_break_start = start;
            iord_break_end = end;
        } else {
            iord_watch_start = start;
            iord_watch_end = end;
        }
    } else if (strcmp(argv[1], "iowr") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            iowr_break_start = start;
            iowr_break_end = end;
        } else {
            iowr_watch_start = start;
            iowr_watch_end = end;
        }
    } else if (strcmp(argv[1], "opfetch") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            opfetch_break_start = start;
            opfetch_break_end = end;
        } else {
            opfetch_watch_start = start;
            opfetch_watch_end = end;
        }
    } else if (strcmp(argv[1], "off") == 0) {
        if (strcmp(argv[0], "break") == 0) {
            memrd_break_start = 0xffff;
            memrd_break_end = 0;
            memwr_break_start = 0xffff;
            memwr_break_end = 0;
            iord_break_start = 0xff;
            iord_break_end = 0;
            iowr_break_start = 0xff;
            iowr_break_end = 0;
            opfetch_break_start = 0xffff;
            opfetch_break_end = 0;
        } else {
            memrd_watch_start = 0xffff;
            memrd_watch_end = 0;
            memwr_watch_start = 0xffff;
            memwr_watch_end = 0;
            iord_watch_start = 0xff;
            iord_watch_end = 0;
            iowr_watch_start = 0xff;
            iowr_watch_end = 0;
            opfetch_watch_start = 0xffff;
            opfetch_watch_end = 0;
        }
    } else {
        printf_P(PSTR("error: unknown type\n"));
    }
}

void cli_bank(int argc, char *argv[])
{
    uint8_t bank;
    if (argc != 2) {
        printf_P(PSTR("usage: bank <0-7>\n"));
        return;
    }
    bank = strtol(argv[1], NULL, 10) & 0x7;
    SET_BANK(bank);
}

void cli_altmon(int argc, char *argv[])
{
    write_mem_P(0xf800, altmon_bin, altmon_bin_len);
    z80_reset(0xf800);
    z80_run();
}

void cli_dboot(int argc, char *argv[])
{
    write_mem_P(0xff00, dbl_bin, dbl_bin_len);
    z80_reset(0xff00);
    z80_run();
}

void cli_sboot(int argc, char *argv[])
{
    write_mem_P(0xff00, simhboot_bin, simhboot_bin_len);
    z80_reset(0xff00);
    z80_run();
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

void cli_fill(int argc, char*argv[]) {
    if (argc != 4) {
        printf_P(PSTR("usage: fill <start> <end> <value>\n"));
        return;
    }
    uint16_t start = strtol(argv[1], NULL, 16) & 0xffff;
    uint16_t end = strtol(argv[2], NULL, 16) & 0xffff;
    uint8_t value = strtol(argv[3], NULL, 16) & 0xff;
    fill_mem(start, end, value);
}

void cli_bus(int argc, char *argv[]) {
    bus_status();
}

void cli_mount(int argc, char *argv[])
{
    if (argc != 3) {
        printf_P(PSTR("usage: mount <drive #> <filename>\n"));
        return;
    }
    uint8_t drv = strtol(argv[1], NULL, 10);
    char *filename = argv[2];
    drive_mount(drv, filename);
}

void cli_unmount(int argc, char *argv[])
{
    if (argc != 2) {
        printf_P(PSTR("usage: unmount <drive #>\n"));
        return;
    }
    uint8_t drv = strtol(argv[1], NULL, 10);
    drive_unmount(drv);
}

void cli_help(int argc, char *argv[]);

typedef struct _cli_entry {
    char *name;
    char *desc;
    void (*func)(int, char*[]);
} cli_entry;

cli_entry cli_cmds[] = {
    {"altmon", "run altmon 8080 monitor", &cli_altmon},
    {"bank", "select active 64K bank", &cli_bank},
    {"bus", "display low-level bus status", &cli_bus},
    {"break", "set breakpoints", &cli_breakwatch},
    {"dboot", "boot disk using Altair disk bootloader", &cli_dboot},
    {"d", "shorthand to continue debugging", &cli_debug},
    {"debug", "debug code at address", &cli_debug},
    {"dir", "shows directory listing", &cli_dir},
    {"dump", "dump memory in hex and ascii", &cli_dump},
    {"fill", "fill memory with byte", &cli_fill},
    {"help", "list available commands", &cli_help},
    {"loadhex", "load intel hex file to memory", &cli_loadhex},
    {"mount", "mount a disk image", &cli_mount},
    {"run", "execute code at address", &cli_run},
    {"reset", "reset the processor, with optional vector", &cli_reset},
    {"savehex", "save intel hex file from memory", &cli_savehex},
    {"sboot", "boot disk using SIMH bootloader", &cli_sboot},
    {"status", "display current status", &cli_status},
    {"s", "shorthand for step", &cli_step},
    {"step", "step processor N cycles", &cli_step},
    {"unmount", "unmount a disk image", &cli_unmount},
    {"watch", "set watch points", &cli_breakwatch}
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