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

#include "hardware/memory.h"
#include "hardware/z80.h"

#include "util/ffwrap.h"
#include "util/pgmspace.h"
#include "util/string.h"

#include "cli.h"
#include "impl.h"

/**
 * Lookup table of monitor commands
 */
const char cli_commands[] PROGMEM =
//  Name          Args                                     Description
    "about\0"     "\0"                                     "z80ctrl copyrights and licenses\0"
    "ascii\0"     "<char>\0"                               "\0"
    "assign\0"    "[port] [r|w|rw] [device]\0"             "assign a device to a port\0"
    "attach\0"    "<port> <r|w> <uart|file> <name>\0"      "attach virtual uart\0"
#if defined(BANK_PORT) || defined(BANK_BASE)
    "base\0"      "<addr>\0"                               "set the base memory address\0"
#endif
    "baud\0"      "<baud>\0"                               "configure UART baud rate\0"
    "bdosdbg\0"   "[on|off]\0"                             "\0"
#ifdef BENCHMARK
    "bench\0"     "\0"                                     "\0"
#endif
    "boot\0"      "[filename]\0"                           "boot from specified disk image\0"
    "bus\0"       "\0"                                     "display low-level bus status\0"
    "break\0"     "[type] [from] [to]\0"                   "set breakpoints\0"
    "c\0"         "[addr]\0"                               "\0"
    "cd\0"        "<path>\0"                               "change directory\0"
    "chdir\0"     "<path>\0"                               "\0"
    "clkdiv\0"    "[divisor]\0"                            "set Z80 clock divider\0"
    "cls\0"       "\0"                                     "clear screen\0"
    "coleco\0"    "<rom> [bios]\0"                         "run a colecovision game\0"
    "copy\0"      "<from> <to>\0"                          "copy a file (alias cp)\0"
    "cp\0"        "<from> <to>\0"                          "\0"
#ifdef USE_RTC
    "date\0"      "[yy-mm-dd] [hh:mm:ss] [wd]\0"           "display or set the date on the rtc\0"
#endif
    "debug\0"     "[addr]\0"                               "debug code at address (alias c)\0"
    "del\0"       "<filename>\0"                           "erase a file (alias era, rm)\0"
    "dir\0"       "[path]\0"                               "shows directory listing in long format\0"
    "disasm\0"    "[addr]\0"                               "disassembles memory location\0"
    "do\0"        "[script]\0"                             "execute a batch file\0"
    "dump\0"      "[addr]\0"                               "dump memory in hex and ascii (alias d)\0"
    "d\0"         "[addr]\0"                               "\0"
    "era\0"       "<filename>\0"                           "\0"
#ifdef SST_FLASH
    "erase\0"     "[addr|all]\0"                           "erase flash ROM\0"
#endif
    "esc\0"       "<codes>+\0"                             "\0"
    "fill\0"      "<from>x<to>x<value>\0"                  "fill memory with byte\0"
#ifdef SST_FLASH
    "flash\0"     "<from>s<filename>x[offset]d[length]\0"  "flash file to ROM\0"
#endif
    "haltsig\0"  "[none|all|reset|halt]\0"                 "configure signals to halt execution\0"
    "haltkey\0"  "<ascii|key>\0"                           "enable halt via keyboard shortcut\0"
    "help\0"     "\0"                                      "list available commands\0"
    "in\0"       "<addr>\0"                                "read a value from a port\0"
    "ioxrd\0"    "<addr> <reg>\0"                          "\0"
    "ioxwr\0"    "<addr> <reg> <value>\0"                  "\0"
    "loadbin\0"  "<from> <filename> [offset] [length]\0"   "load binary file to memory\0"
    "loadhex\0"  "<filename>\0"                            "load intel hex file to memory\0"
    "ls\0"       "<path>\0"                                "shows directory listing in wide format\0"
    "md\0"       "<path>\0"                                "\0"
    "mkdir\0"    "<path>\0"                                "create a subdirectory (alias md)\0"
    "mount\0"    "<drive> <filename>\0"                    "mount a disk image\0"
    "move\0"     "<from> <to>\0"                           "\0"
    "mv\0"       "<from> <to>\0"                           "\0"
    "out\0"      "<addr> <value>+\0"                       "write a value to a port\0"
    "poke\0"     "<addr> [values]+\0"                      "poke values into memory\0"
    "rd\0"       "<path>\0"                                "\0"
    "ren\0"      "<from> <to>\0"                           "rename/move a file or directory (alias mv)\0"
    "rm\0"       "<path>\0"                                "\0"
    "rmdir\0"    "<path>\0"                                "\0"
    "run\0"      "[addr]\0"                                "execute code at address\0"
    "reset\0"    "[addr]\0"                                "\0"
    "savebin\0"  "<from> <to> <file>\0"                    "save binary file from memory\0"
    "savehex\0"  "<from> <to> <file>\0"                    "save intel hex file from memory\0"
    "screen\0"   "[width] [height]\0"                      "set screen size\0"
    "s\0"        "\0"                                      "\0"
    "step\0"     "\0"                                      "step processor N cycles (alias s)\0"
#ifdef TMS_BASE
    "tmsreg\0"   "\0"                                      "report tms registers\0"
    "tmsdump\0"  "[from] [to]\0"                           "dump tms memory in hex and ascii\0"
    "tmsfill\0"  "<from> <to> <value>\0"                   "fill tms memory with byte\0"
    "tmslbin\0"  "<from> <filename> [offset] [length]\0"   "load binary file to tms memory\0"
#endif
    "unmount\0"  "<drive>\0"                               "unmount a disk image\0"
    "watch\0"    "[type] [from] [to]\0"                    "set watch points\0"
    "xmrx\0"     "[filename]\0"                            "receive file(s) via x/ymodem\0"
    "xmtx\0"     "<filename>...\0"                         "send file(s) via x/ymodem\0";

void cli_help(int argc, char* argv[]);

/**
 * Lookup table of function pointers for monitor commands
 */
void* const cli_functions[] PROGMEM = {
    &cli_about,
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
    &cli_debug, // c
    &cli_chdir, // cd
    &cli_chdir,
    &cli_clkdiv,
    &cli_cls,
    &cli_coleco,
    &cli_copy, // copy
    &cli_copy, // cp
#ifdef USE_RTC
    &cli_date,
#endif
    &cli_debug,
    &cli_del, // del
    &cli_dir,
    &cli_disasm,
    &cli_do,
    &cli_dump,
    &cli_dump, // d
    &cli_del, // era
#ifdef SST_FLASH
    &cli_erase,
#endif
    &cli_esc, // esc
    &cli_fill,
#ifdef SST_FLASH
    &cli_loadbin, // flash
#endif
    &cli_halt,
    &cli_haltkey,
    &cli_help,
    &cli_in,
    &cli_ioxread,
    &cli_ioxwrite,
    &cli_loadbin,
    &cli_loadhex,
    &cli_dir, // ls
    &cli_mkdir, // md
    &cli_mkdir,
    &cli_mount,
    &cli_ren, // move
    &cli_ren, // mv
    &cli_out,
    &cli_poke,
    &cli_del, // rd
    &cli_ren,
    &cli_del, // rm
    &cli_del, // rmdir
    &cli_run,
    &cli_reset,
    &cli_savebin,
    &cli_savehex,
    &cli_screen,
    &cli_step, // s
    &cli_step,
#ifdef TMS_BASE
    &cli_tmsreg, // tmsreg
    &cli_dump, // tmsdump
    &cli_fill, // tmsfill
    &cli_loadbin, // tmslbin
#endif
    &cli_unmount,
    &cli_breakwatch,
    &cli_xmrx,
    &cli_xmtx
};

/**
 * Display license and copyright information
 */
void cli_about(int argc, char* argv[])
{
    printf_P(PSTR("%S\n%S\n"), z80ctrl_banner, z80ctrl_copyright);
}

typedef struct {
    int ord;
    const char* name;
    const char* args;
    const char* desc;
    const char* next;
} cli_command_t;

/**
 * Given a starting pointer to a command in a packed string array, return pointers
 * to the name, args, description, and next command in the array. cmd.next will
 * be null if there is no next command. If the starting pointer points to an empty
 * string, null pointers will be returned for everything.
 */
cli_command_t cli_parse_command(const char* start)
{
    cli_command_t cmd;
    cmd.ord = -1;
    cmd.name = NULL;
    cmd.args = NULL;
    cmd.desc = NULL;
    cmd.next = NULL;
    if (strlen_P(start) != 0) {
        cmd.name = start;
        cmd.args = cmd.name + strlen_P(cmd.name) + 1;
        cmd.desc = cmd.args + strlen_P(cmd.args) + 1;
        cmd.next = cmd.desc + strlen_P(cmd.desc) + 1;
        if (strlen_P(cmd.next) == 0)
            cmd.next = NULL;
    }
    return cmd;
}

/**
 * List available commands with help text
 */
void cli_help(int argc, char* argv[])
{
    cli_command_t cmd;
    cmd.next = cli_commands;
    while (cmd.next != NULL) {
        cmd = cli_parse_command(cmd.next);
        if (strlen_P(cmd.desc) != 0) // hide commands with no description
            printf_P(PSTR("%S %S\n\t%S\n"), cmd.name, cmd.args, cmd.desc);
    }
}

/**
 * Run a .PRG file (binary with start address)
 */
uint8_t cli_runprg(int argc, char* argv[])
{
    char filename[256];
    FILINFO fno;
    strcpy(filename, argv[0]);
    if (!endswith(filename, PSTR(".prg")))
        strcat_P(filename, PSTR(".prg"));
    if (f_stat(filename, NULL) == FR_OK) {
        file_dma_save_cli(argc, argv);
        uint16_t start = mem_loadbin(mem_write_banked, filename, -1, 0, 0);
        z80_reset(start);
        z80_run();
        putchar('\n');
        return 1;
    } else {
        return 0;
    }
}

cli_command_t cli_find_command(const char* commands, const char* name)
{
    cli_command_t cmd;
    cmd.next = commands;
    for (int i = 0; cmd.next != NULL; i++) {
        cmd = cli_parse_command(cmd.next);
        if (strcmp_P(name, cmd.name) == 0) {
            cmd.ord = i;
            return cmd;
        }
    }
    cmd.ord = -1;
    cmd.name = NULL;
    cmd.args = NULL;
    cmd.desc = NULL;
    cmd.next = NULL;
    return cmd;
}

/**
 * Dispatch a single command
 */
void cli_dispatch(char* buf)
{
    char* argv[MAXARGS];
    int argc;
    for (int i = 0; i < strlen(buf); i++)
        buf[i] = tolower(buf[i]);
    if ((argv[0] = strtok(buf, WHITESPACE)) == NULL)
        return; // empty command
    cli_command_t cmd = cli_find_command(cli_commands, argv[0]);
    if (cmd.name != NULL) {
        // printf_P(PSTR("command found at %d: %S %S: %S\n"), cmd.ord, cmd.name, cmd.args, cmd.desc);
        void (*cmd_func)(int, char*[]);
        cmd_func = pgm_read_ptr(&cli_functions[cmd.ord]);
        for (argc = 1; argc < MAXARGS; argc++) {
            if ((argv[argc] = strtok(NULL, WHITESPACE)) == NULL)
                break;
        }
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
    printf_P(PSTR("\n\ntype help to list available commands; about for credits\n"));
    cli_exec(AUTOEXEC);
    cli_loop();
}