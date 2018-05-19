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
#include "util.h"
#include "disasm.h"
#include "diskemu.h"
#include "diskio.h"

FATFS fs;

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
            printf_P(PSTR("error opening file: %S\n"), strlookup(fr_text, fr));
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
        } else
            printf_P(PSTR("error: %S on line %d\n"), strlookup(ihex_rc_text, res.rc), line);
    }
    if (filename != NULL)
        if ((fr = f_close(&fp)) != FR_OK) {
            printf_P(PSTR("error closing file: %S\n"), strlookup(fr_text, fr));
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
            printf_P(PSTR("error opening file: %S\n"), strlookup(fr_text, fr));
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
            printf_P(PSTR("error closing file: %s\n"), strlookup(fr_text, fr));
            return;
        }
    }
}

void cli_disasm(int argc, char *argv[])
{
    uint16_t start, end;
    if (argc < 2) {
        printf_P(PSTR("usage: disasm <start> [end]\n"));
        return;
    }
    start = strtol(argv[1], NULL, 16) & 0xffff;
    if (argc < 3)
        end = start + 0xf;
    else
        end = strtol(argv[2], NULL, 16) & 0xffff;
    disasm_mem(start, end);
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
    printf_P(PSTR("%04x %04x\n"), start, end);
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
    z80_debug(0);
}

void cli_step(int argc, char *argv[])
{
    uint32_t cycles = 1;
    if (argc >= 2)
        cycles = strtol(argv[1], NULL, 10);            
    z80_debug(cycles);
}

void cli_breakwatch(int argc, char *argv[])
{
    range *ranges;
    uint8_t type;

    if (strcmp_P(argv[0], PSTR("break")) == 0)
        ranges = breaks;
    else
        ranges = watches;
    
    // If no parameters given, show current status
    if (argc == 1) {
        printf_P(PSTR("%s status:\n"), argv[0]);
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            if (!ENABLED(ranges, i))
                printf_P(PSTR("\t%S: disabled\n"), strlookup(debug_names, i));
            else
                printf_P(PSTR("\t%S: %04x-%04x\n"), strlookup(debug_names, i), ranges[i].start, ranges[i].end);
        }
        printf_P(PSTR("\nusage:\n\t%s <type> [start] [end]\n"), argv[0]);
        printf_P(PSTR("\t%s <type> off to disable type\n"), argv[0]);
        printf_P(PSTR("\t%s off to disable all\n"), argv[0]);
        return;
    }
    if (strcmp_P(argv[1], PSTR("off")) == 0) {
        // turn off all ranges
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            ranges[i].start = 0xffff;
            ranges[i].end = 0;
        }
        return;
    } else {
        // find the debugging type that the user specified
        for (type = 0; type < DEBUGCNT; type++)
            if (strcmp_P(argv[1], strlookup(debug_names, type)) == 0)
                break;
        if (type == DEBUGCNT) {
            printf_P(PSTR("error: unknown type\n"));
            return;
        }
        if (argc == 2) {
            // no range specified, enable for 0x0000-0xffff
            ranges[type].start = 0;
            ranges[type].end = 0xffff;
        } else if (argc >= 3) {
            if (strcmp_P(argv[2], PSTR("off")) == 0) {
                ranges[type].start = 0xffff;
                ranges[type].end = 0;
            } else {
                // get starting address
                ranges[type].start = strtol(argv[2], NULL, 16);
                if (argc >= 4)
                    // get ending address if specified
                    ranges[type].end = strtol(argv[3], NULL, 16);
                else
                    // if no ending address, start and end are the same
                    ranges[type].end = ranges[type].start;
            }
        }
    }
}

#ifdef SET_BANK
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
#endif

void cli_altmon(int argc, char *argv[])
{
    write_mem_P(0xf800, altmon_bin, altmon_bin_len);
    z80_reset(0xf800);
    z80_run();
}

void cli_dboot(int argc, char *argv[])
{
    write_mem_P(0xff00, dbl_bin, dbl_bin_len);
    if (argc <= 1 || strcmp(argv[1], "-l")) {
        z80_reset(0xff00);
        z80_run();
    }
}

void cli_sboot(int argc, char *argv[])
{
    write_mem_P(0xff00, simhboot_bin, simhboot_bin_len);
    if (argc <= 1 || strcmp(argv[1], "-l")) {
        z80_reset(0xff00);
        z80_run();
    }
}

void cli_dir(int argc, char *argv[])
{
 	FRESULT fr;
    FILINFO finfo;
    DIR dir;
	UINT cnt;
    FATFS *fs;
    char fname[14];

    if (argc == 1)
        argv[1] = "/";

    fr = f_opendir(&dir, argv[1]);
    if (fr) {
        printf_P(PSTR("error reading directory: %S\n"), strlookup(fr_text, fr));
        return;
    }
    cnt = 0;
    for(;;) {
        fr = f_readdir(&dir, &finfo);
        if (fr != FR_OK) { 
            printf_P(PSTR("error reading directory: %S\n"), strlookup(fr_text, fr)); 
            break; 
        }
        if ((cnt % 5 == 0 && cnt != 0) || !finfo.fname[0])
            printf_P(PSTR("\n"));
        if (!finfo.fname[0]) break;
        strcpy(fname, finfo.fname);
        if (finfo.fattrib & AM_DIR)
            strcat(fname, "/");
        printf_P(PSTR("%-14s"), fname);
        cnt++;
    }
    if (fr)
        printf_P(PSTR("error reading directory: %S\n"), strlookup(fr_text, fr));
    else
        printf_P(PSTR("%u item(s)\n"), cnt);
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
    z80_buslog(bus_status());
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

void cli_cls(int argc, char *argv[])
{
    printf_P(PSTR("\e[2J"));
}

void cli_help(int argc, char *argv[]);

const char cli_cmd_names[] PROGMEM = 
    "altmon\0"
#ifdef SET_BANK
    "bank\0"
#endif
    "bus\0"
    "break\0"
    "c\0"
    "cls\0"
    "dboot\0"
    "debug\0"
    "dir\0"
    "disasm\0"
    "dump\0"
    "fill\0"
    "help\0"
    "loadhex\0"
    "mount\0"
    "run\0"
    "reset\0"
    "savehex\0"
    "sboot\0"
    "s\0"
    "step\0"
    "unmount\0"
    "watch";

const char cli_cmd_help[] PROGMEM =
    "run altmon 8080 monitor\0"                     // altmon
#ifdef SET_BANK
    "select active 64K bank\0"                      // bank
#endif
    "display low-level bus status\0"                // bus
    "set breakpoints\0"                             // break
    "shorthand to continue debugging\0"             // c
    "clear screen\0"                                // cls
    "boot disk using Altair disk bootloader\0"      // dboot
    "debug code at address\0"                       // debug
    "shows directory listing\0"                     // dir
    "disassembles memory location\0"                // disasm
    "dump memory in hex and ascii\0"                // dump
    "fill memory with byte\0"                       // fill
    "list available commands\0"                     // help
    "load intel hex file to memory\0"               // loadhex
    "mount a disk image\0"                          // mount
    "execute code at address\0"                     // run
    "reset the processor, with optional vector\0"   // reset
    "save intel hex file from memory\0"             // savehex
    "boot disk using SIMH bootloader\0"             // sboot
    "shorthand for step\0"                          // s
    "step processor N cycles\0"                     // step
    "unmount a disk image\0"                        // unmount
    "set watch points";                             // watch

void * const cli_cmd_functions[] PROGMEM = {
    &cli_altmon,
#ifdef SET_BANK
    &cli_bank,
#endif
    &cli_bus,
    &cli_breakwatch,
    &cli_debug,
    &cli_cls,
    &cli_dboot,
    &cli_debug,
    &cli_dir,
    &cli_disasm,
    &cli_dump,
    &cli_fill,
    &cli_help,
    &cli_loadhex,
    &cli_mount,
    &cli_run,
    &cli_reset,
    &cli_savehex,
    &cli_sboot,
    &cli_step,
    &cli_step,
    &cli_unmount,
    &cli_breakwatch
};

#define NUM_CMDS (sizeof(cli_cmd_functions)/sizeof(void *))

void cli_help(int argc, char *argv[])
{
    int i;
    printf_P(PSTR("available commands:\n"));
    for (i = 0; i < NUM_CMDS; i++) {
        printf_P(PSTR("%S\t%S\n"), strlookup(cli_cmd_names, i), strlookup(cli_cmd_help, i));
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
    void (*cmd_func)(int, char*[]);

    disk_initialize(0);
    if ((fr = f_mount(&fs, "", 1)) != FR_OK)
        printf_P(PSTR("error mounting drive: %S\n"), strlookup(fr_text, fr));

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
                if (strcmp_P(argv[0], strlookup(cli_cmd_names, i)) == 0) {
                    cmd_func = pgm_read_ptr(&cli_cmd_functions[i]);
                    break;
                }
            }
            if (i == NUM_CMDS)
                printf_P(PSTR("unknown command: %s. type help for list.\n"), argv[0]);
            else
                cmd_func(argc, argv);
        }
    }
}