#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "altmon.h"
#include "bus.h"
#include "memory.h"
#include "z80.h"
#include "cli.h"
#include "ff.h"

FATFS fs;

void cli_loadihex(char *argv[], int argc)
{
    int rc;
    char buf[512+11];
    do {
        gets(buf);
        rc = write_ihex_rec(buf);
        if (rc < 0 && rc != IHEX_EOF)
            printf_P(PSTR("error %d reading ihex\n"), rc);
    } while (rc != IHEX_EOF);
}

void cli_saveihex(char *argv[], int argc)
{

    uint16_t addr;
    uint16_t length;
    uint16_t i;
    if (argc != 3) {
        printf_P(PSTR("usage: saveihex <start> <length>\n"));
        return;
    }
    sscanf(argv[1], "%x", &addr);
    sscanf(argv[2], "%x", &length);
    for (;;) {
        if (length > 16) {
            printf_P(PSTR("%s\n"), read_ihex_rec(addr, 16));
            length -= 16;
            addr += 16;
        } else {
            printf_P(PSTR("%s\n"), read_ihex_rec(addr, length));
            break;
        }
    }
    printf_P(PSTR("%s\n"), read_ihex_rec(0,0));
}

void cli_dump(char *argv[], int argc)
{
    uint16_t addr;
    uint16_t length;
    if (argc != 3) {
        printf_P(PSTR("usage: dump <start> <length>\n"));
        return;
    }
    sscanf(argv[1], "%x", &addr);
    sscanf(argv[2], "%x", &length);
    dump_mem(addr, length);
}

void cli_run(char *argv[], int argc)
{
    uint16_t addr;
    if (argc != 2) {
        printf_P(PSTR("usage: run <address>\n"));
        return;
    }
    sscanf(argv[1], "%x", &addr);
    z80_run(addr);
}

void cli_bus(char *argv[], int argc)
{
    puts(bus_status());
}

void cli_altmon(char *argv[], int argc)
{
    write_mem_P(0xf800, altmon_bin, altmon_bin_len);
    z80_run(0xf800);
}

void cli_ls(char *argv[], int argc)
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
        printf_P(PSTR("error %d\n"), fr); 
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
        printf_P(PSTR("error %d\n"), fr);
    };
}

void cli_help(char *argv[], int argc);

typedef struct _cli_entry {
    char *name;
    char *desc;
    void (*func)(char **, int);
} cli_entry;

cli_entry cli_cmds[] = {
    {"help", "list available commands", &cli_help},
    {"dump", "hex dump of memory range", &cli_dump},
    {"run", "execute code at address", &cli_run},
    {"bus", "display current bus status", &cli_bus},
    {"altmon", "run altmon 8080 monitor", &cli_altmon},
    {"loadhex", "load Intel HEX records to memory", &cli_loadihex},
    {"savehex", "save Intel HEX records from memory", &cli_saveihex},
    {"ls", "shows a directory listing", &cli_ls}
};

#define NUM_CMDS (sizeof(cli_cmds)/sizeof(cli_entry))

void cli_help(char *argv[], int argc)
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
        printf_P(PSTR("error %d mounting fs\n"), fr);
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
                    cli_cmds[i].func(argv, argc);
                    break;
                }
            }
            if (i == NUM_CMDS) {
                printf_P(PSTR("unknown command: %s. type help for list.\n"), argv[0]);
            }
        }
    }
}
