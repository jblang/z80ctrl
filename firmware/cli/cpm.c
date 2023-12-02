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
 * @file cpm.c CP/M-related commands
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fatfs/ff.h"

#include "emulation/bdos.h"
#include "emulation/disk.h"
#include "emulation/sio.h"

#include "hardware/memory.h"
#include "hardware/z80.h"

#include "util/ffwrap.h"
#include "util/pgmspace.h"
#include "util/string.h"

#include "cli.h"

/**
 * Boot from a disk
 */
void cli_boot(int argc, char* argv[])
{
    if (argc == 2) {
        drive_mount(0, argv[1]);
    }
    if (drive_bootload()) {
        z80_reset(0);
        z80_run();
    }
}

/**
 * Attach a virtual UART to a physical one
 */
void cli_attach(int argc, char* argv[])
{
    uint8_t dir;
    uint8_t mode = SIO_UNATTACHED;
    char* filename = NULL;
    if (argc != 4) {
        printf_P(PSTR("usage: attach [0|1] [read|write] [uart0|uart1|filename]\n"));
        return;
    }
    if (strcmp_P(argv[2], PSTR("read")) == 0)
        dir = SIO_INPUT;
    else
        dir = SIO_OUTPUT;

    uint8_t virtual = strtoul(argv[1], NULL, 10) & 1;
    if (strcmp_P(argv[3], PSTR("uart0")) == 0)
        mode = SIO_UART0;
    else if (strcmp_P(argv[3], PSTR("uart1")) == 0)
        mode = SIO_UART1;
    else {
        mode = SIO_FILE;
        filename = argv[3];
    }
    sio_attach(virtual, dir, mode, filename);
}

/**
 * Mount a disk image
 */
void cli_mount(int argc, char* argv[])
{
    if (argc != 3) {
        printf_P(PSTR("usage: mount <drive #> <filename>\n"));
        return;
    }
    uint8_t drv = strtoul(argv[1], NULL, 10);
    char* filename = argv[2];
    drive_mount(drv, filename);
}

/**
 * Unmount a disk image
 */
void cli_unmount(int argc, char* argv[])
{
    if (argc != 2) {
        printf_P(PSTR("usage: unmount <drive #>\n"));
        return;
    }
    uint8_t drv = strtoul(argv[1], NULL, 10);
    drive_unmount(drv);
}

/**
 * Enable/disable BDOS debug messages
 */
void cli_bdosdbg(int argc, char* argv[])
{
    if (argc == 2) {
        if (strcmp_P(argv[1], PSTR("on")) == 0)
            bdos_debug = 1;
        else if (strcmp_P(argv[1], PSTR("off")) == 0)
            bdos_debug = 0;
    }
    if (bdos_debug)
        printf("bdos debug is enabled\n");
    else
        printf("bdos debug is disabled\n");
}

/**
 * Run a CP/M .COM file
 */
uint8_t cli_runcom(int argc, char* argv[])
{
    char filename[256];
    strcpy(filename, argv[0]);
    if (!endswith(filename, PSTR(".com")))
        strcat_P(filename, PSTR(".com"));
    if (f_stat(filename, NULL) == FR_OK) {
        // Look for BDOS runtime file in current directory then root directory
        uint16_t start;
        if (f_stat("bdos.bin", NULL) == FR_OK) {
            start = mem_loadbin(mem_write_banked, "bdos.bin", -1, 0, 0);
        } else if (f_stat("/bdos.bin", NULL) == FR_OK) {
            start = mem_loadbin(mem_write_banked, "/bdos.bin", -1, 0, 0);
        } else {
            // Last resort: warn and try to run the program without bdos loaded
            printf_P(PSTR("warning: bdos.bin not found in current or root directory.\n"));
            start = 0x100;
        }
        bdos_init(argc, argv);
        mem_loadbin(mem_write_banked, filename, 0x100, 0, 0);
        z80_reset(start);
        z80_run();
        putchar('\n');
        return 1;
    } else {
        return 0;
    }
}
