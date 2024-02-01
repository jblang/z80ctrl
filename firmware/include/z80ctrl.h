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
 * @file cli.c public command-line functions
 */

#ifndef CLI_H
#define CLI_H
#include <stdint.h>

#define WHITESPACE " \t\r\n"
#define MAXBUF 256
#define MAXARGS 256
#define AUTOEXEC "autoexec.z8c"

/* bench.c */
void cli_bench(int argc, char* argv[]);

/* cli.c */
void cli_dispatch(char* buf);
void cli_about(int argc, char *argv[]);
void cli_help(int argc, char *argv[]);
void cli_start(void);

/* coleco.c */
void cli_coleco(int argc, char* argv[]);

/* cpm.c */
void cli_boot(int argc, char* argv[]);
void cli_attach(int argc, char* argv[]);
void cli_mount(int argc, char* argv[]);
void cli_unmount(int argc, char* argv[]);
void cli_bdosdbg(int argc, char* argv[]);
uint8_t cli_runcom(int argc, char* argv[]);

extern uint8_t clibuf[256];

/* file.c */
void cli_chdir(int argc, char* argv[]);
void cli_mkdir(int argc, char* argv[]);
void cli_dir(int argc, char* argv[]);
void cli_copy(int argc, char* argv[]);
void cli_ren(int argc, char* argv[]);
void cli_del(int argc, char* argv[]);
void cli_loadhex(int argc, char* argv[]);
void cli_savehex(int argc, char* argv[]);
void cli_loadbin(int argc, char* argv[]);
void cli_savebin(int argc, char* argv[]);
void cli_xmrx(int argc, char* argv[]);
void cli_xmtx(int argc, char* argv[]);
void cli_exec(char* filename);
void cli_do(int argc, char* argv[]);

/* io.c */
void cli_assign(int argc, char* argv[]);
void cli_out(int argc, char* argv[]);
void cli_in(int argc, char* argv[]);
void cli_tmsreg(int argc, char* argv[]);

/* hardware.c */
void cli_baud(int argc, char* argv[]);
void cli_date(int argc, char* argv[]);
void cli_ioxread(int argc, char* argv[]);
void cli_ioxwrite(int argc, char* argv[]);

/* memory.c */
void cli_base(int argc, char* argv[]);
void cli_dump(int argc, char* argv[]);
void cli_fill(int argc, char* argv[]);
void cli_poke(int argc, char* argv[]);
void cli_erase(int argc, char* argv[]);

/* term.c */
void cli_screen(int argc, char* argv[]);
void cli_cls(int argc, char* argv[]);
void cli_esc(int argc, char* argv[]);
void cli_ascii(int argc, char* argv[]);
void cli_haltkey(int argc, char* argv[]);

extern uint8_t screenwidth;
extern uint8_t screenheight;

/* z80.c */
void cli_disasm(int argc, char* argv[]);
void cli_clkdiv(int argc, char* argv[]);
void cli_reset(int argc, char* argv[]);
void cli_run(int argc, char* argv[]);
void cli_debug(int argc, char* argv[]);
void cli_step(int argc, char* argv[]);
void cli_bus(int argc, char* argv[]);
void cli_halt(int argc, char* argv[]);
void cli_breakwatch(int argc, char* argv[]);

extern const char z80ctrl_banner[];
extern const char z80ctrl_copyright[];
#endif