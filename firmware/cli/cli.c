/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
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

/**
 * @file cli.c Command-line interface
 */

#include <avr/interrupt.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emulation/filedma.h"

#include "hardware/z80.h"

#include "util/ffwrap.h"
#include "util/pgmspace.h"
#include "util/string.h"

#include "cli.h"
#include "impl.h"

/**
 * Lookup table of monitor command names
 */
const char cli_cmd_names[] PROGMEM = 
    "ascii\0"
    "assign\0"
    "attach\0"
#if defined(BANK_PORT) || defined(BANK_BASE)
    "base\0"
#endif
    "baud\0"
    "bdosdbg\0"
#ifdef BENCHMARK
    "bench\0"
#endif
    "boot\0"
    "bus\0"
    "break\0"
    "c\0"
    "cd\0"
    "chdir\0"
    "clkdiv\0"
    "cls\0"
    "coleco\0"
    "copy\0"
    "cp\0"
#ifdef USE_RTC
    "date\0"
#endif
    "debug\0"
    "del\0"
    "dir\0"
    "disasm\0"
    "do\0"
    "dump\0"
    "d\0"
    "era\0"
#ifdef SST_FLASH
    "erase\0"
#endif
    "esc\0"
    "fill\0"
#ifdef SST_FLASH
    "flash\0"
#endif
    "halt\0"
    "haltkey\0"
    "help\0"
    "in\0"
    "ioxrd\0"
    "ioxwr\0"
    "loadbin\0"
    "loadhex\0"
    "ls\0"
    "md\0"
    "mkdir\0"
    "mount\0"
    "move\0"
    "mv\0"
    "out\0"
    "poke\0"
    "rd\0"
    "ren\0"
    "rm\0"
    "rmdir\0"
    "run\0"
    "reset\0"
    "savebin\0"
    "savehex\0"
    "screen\0"
    "s\0"
    "step\0"
#ifdef TMS_BASE
    "tmsreg\0"
    "tmsdump\0"
    "tmsfill\0"
    "tmslbin\0"
#endif
    "unmount\0"
    "watch\0"
    "xmrx\0"
    "xmtx";

/**
 * Lookup table of help text for monitor commands
 */
const char cli_cmd_help[] PROGMEM =
    "\0"                                            // ascii
    "assign a device to a port\0"                   // assign
    "attach virtual uart\0"                         // attach
#if defined(BANK_PORT) || defined(BANK_BASE)
    "set the base memory address\0"                 // base
#endif
    "configure UART baud rate\0"                    // baud
    "\0"                                            // bdosdbg
#ifdef BENCHMARK
    "\0"                                            // bench
#endif
    "boot from specified disk image\0"              // boot
    "display low-level bus status\0"                // bus
    "set breakpoints\0"                             // break
    "\0"                                            // c
    "change directory\0"                            // cd
    "\0"                                            // chdir
    "set Z80 clock divider\0"                       // clkdiv
    "clear screen\0"                                // cls
    "run a colecovision game\0"                       // coleco
    "copy a file (alias cp)\0"                      // copy
    "\0"                                            // cp
#ifdef USE_RTC
    "display or set the date on the rtc\0"          // date
#endif
    "debug code at address (alias c)\0"             // debug
    "erase a file (alias era, rm)\0"                // del
    "shows directory listing in long format\0"      // dir
    "disassembles memory location\0"                // disasm
    "execute a batch file\0"                        // do
    "dump memory in hex and ascii (alias d)\0"      // dump
    "\0"                                            // d
    "\0"                                            // era
#ifdef SST_FLASH
    "erase flash ROM\0"                             // erase
#endif
    "\0"                                            // esc
    "fill memory with byte\0"                       // fill
#ifdef SST_FLASH
    "flash file to ROM\0"                           // flash
#endif
    "enable or disable halt\0"                      // halt
    "enable halt via keyboard shortcut\0"           // haltkey
    "list available commands\0"                     // help
    "read a value from a port\0"                    // in
    "\0"                                            // ioxread
    "\0"                                            // ioxwrite
    "load binary file to memory\0"                  // loadbin
    "load intel hex file to memory\0"               // loadhex
    "shows directory listing in wide format\0"      // ls
    "\0"                                            // md
    "create a subdirectory (alias md)\0"            // mkdir
    "mount a disk image\0"                          // mount
    "\0"                                            // move
    "\0"                                            // mv
    "write a value to a port\0"                     // out
    "poke values into memory\0"                     // poke
    "\0"                                            // rd
    "rename/move a file or directory (alias mv)\0"  // ren
    "\0"                                            // rm
    "\0"                                            // rmdir
    "execute code at address\0"                     // run
    "\0"                                            // reset
    "save binary file from memory\0"                // savebin
    "save intel hex file from memory\0"             // savehex
    "set screen size\0"                             // screen
    "\0"                                            // s
    "step processor N cycles (alias s)\0"           // step
#ifdef TMS_BASE
    "report tms registers\0"                        // tmsreg
    "dump tms memory in hex and ascii\0"            // tmsdump
    "fill tms memory with byte\0"                   // tmsfill
    "load binary file to tms memory\0"              // tmslbin
#endif
    "unmount a disk image\0"                        // unmount
    "set watch points\0"                            // watch
    "receive a file via xmodem\0"                   // xmrx
    "send a file via xmodem";                       // xmtx

void cli_help(int argc, char *argv[]);

/**
 * Lookup table of function pointers for monitor commands
 */
void * const cli_cmd_functions[] PROGMEM = {
    &cli_ascii,
    &cli_assign,
    &cli_attach,
#if defined(BANK_PORT) || defined(BANK_BASE)
    &cli_base,
#endif
    &cli_baud,
    &cli_bdosdbg,
#ifdef BENCHMARK
    &cli_bench,
#endif
    &cli_boot,
    &cli_bus,
    &cli_breakwatch,
    &cli_debug,     // c
    &cli_chdir,     // cd
    &cli_chdir,
    &cli_clkdiv,
    &cli_cls,
    &cli_coleco,
    &cli_copy,       // copy
    &cli_copy,       // cp
#ifdef USE_RTC
    &cli_date,
#endif
    &cli_debug,
    &cli_del,       // del
    &cli_dir,
    &cli_disasm,
    &cli_do,
    &cli_dump,
    &cli_dump,      // d
    &cli_del,       // era
#ifdef SST_FLASH
    &cli_erase,
#endif
    &cli_esc,       // esc
    &cli_fill,
#ifdef SST_FLASH
    &cli_loadbin,   // flash
#endif
    &cli_halt,
    &cli_haltkey,
    &cli_help,
    &cli_in,
    &cli_ioxread,
    &cli_ioxwrite,
    &cli_loadbin,
    &cli_loadhex,
    &cli_dir,       // ls
    &cli_mkdir,     // md
    &cli_mkdir,
    &cli_mount,
    &cli_ren,        // move
    &cli_ren,        // mv
    &cli_out,
    &cli_poke,
    &cli_del,       // rd
    &cli_ren,
    &cli_del,       // rm
    &cli_del,       // rmdir
    &cli_run,
    &cli_reset,
    &cli_savebin,
    &cli_savehex,
    &cli_screen,
    &cli_step,      // s
    &cli_step,
#ifdef TMS_BASE
    &cli_tmsreg,    // tmsreg
    &cli_dump,      // tmsdump
    &cli_fill,      // tmsfill
    &cli_loadbin,   // tmslbin
#endif
    &cli_unmount,
    &cli_breakwatch,
    &cli_xmrx,
    &cli_xmtx
};

#define NUM_CMDS (sizeof(cli_cmd_functions)/sizeof(void *))

/**
 * List available commands with help text
 */
void cli_help(int argc, char *argv[])
{
    int i;
    printf_P(PSTR("available commands:\n"));
    for (i = 0; i < NUM_CMDS; i++) {
        if (strlen_P(strlookup(cli_cmd_help, i)))
            printf_P(PSTR("%S\t%S\n"), strlookup(cli_cmd_names, i), strlookup(cli_cmd_help, i));
    }
}

/**
 * Run a .PRG file (binary with start address)
 */
uint8_t cli_runprg(int argc, char *argv[])
{
    char filename[256];
    FILINFO fno;
    strcpy(filename, argv[0]);
    if (!endswith(filename, PSTR(".prg")))
        strcat_P(filename, PSTR(".prg"));
    if (f_stat(filename, NULL) == FR_OK) {
        file_dma_save_cli(argc, argv);
        uint16_t start = ffw_loadbin(filename, MEM, -1, 0, 0);
        z80_reset(start);
        z80_run();
        putchar('\n');
        return 1;
    } else {
        return 0;
    }
}

/**
 * Dispatch a single command
 */
void cli_dispatch(char *buf) 
{
    char *cmd;
    char *argv[MAXARGS];
    int argc;
    int i;
    void (*cmd_func)(int, char*[]);
    if ((argv[0] = strtok(buf, WHITESPACE)) == NULL)
        return;
    for (argc = 1; argc < MAXARGS; argc++) {
        if ((argv[argc] = strtok(NULL, WHITESPACE)) == NULL)
            break;
    }
    for (i = 0; i < strlen(argv[0]); i++)
        argv[0][i] = tolower(argv[0][i]);
    for (i = 0; i < NUM_CMDS; i++) {
        if (strcmp_P(argv[0], strlookup(cli_cmd_names, i)) == 0) {
            cmd_func = pgm_read_ptr(&cli_cmd_functions[i]);
            break;
        }
    }
    if (i < NUM_CMDS) {
        cmd_func(argc, argv);
    } else if (!cli_runcom(argc, argv)) {
        if (!cli_runprg(argc, argv)) {
            printf_P(PSTR("unknown command: %s. type help for list.\n"), argv[0]);
        }
    }
}

/**
 * Main command-line loop for monitor
 */
void cli_loop(void)
{
 	FRESULT fr;
    char buf[MAXBUF];
    for (;;) {
        fr = f_getcwd(buf, MAXBUF);
        if (fr)
            buf[0] = '\0';
        printf_P(PSTR("z80ctrl:%s>"), buf);
        if (fgets(buf, sizeof buf - 1, stdin) != NULL)
            cli_dispatch(buf);
    }
}

void cli_start(void)
{
    printf_P(PSTR("\n\ntype help to list available commands\n"));
    cli_exec(AUTOEXEC);
    cli_loop();
}