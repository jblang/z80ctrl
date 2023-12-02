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
 * @file cpm.c I/O-related commands
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/io.h"
#include "hardware/tms9918.h"

#include "util/pgmspace.h"

/**
 * Assign a device to a port
 */
void cli_assign(int argc, char* argv[])
{
    if (argc < 3) {
        iorq_list();
        printf_P(PSTR("\n\nusage: %s <port> <r|w|rw> <device>\n"), argv[0]);
        return;
    }
    uint8_t port = strtoul(argv[1], NULL, 16) & 0xff;
    uint8_t mode;
    if (strcmp_P(argv[2], PSTR("r")) == 0)
        mode = IORQ_READ;
    else if (strcmp_P(argv[2], PSTR("w")) == 0)
        mode = IORQ_WRITE;
    else if (strcmp_P(argv[2], PSTR("rw")) == 0)
        mode = IORQ_RW;
    else {
        printf_P(PSTR("usage: %s <port> <r|w|rw> <device>\n"), argv[0]);
        return;
    }
    uint8_t device = iorq_deviceid(argv[3]);
    if (device == DEV_INVALID) {
        printf_P(PSTR("error: invalid device name\n"));
        return;
    }

    if (iorq_assign(port, mode, device) != device)
        printf_P(PSTR("error: port in use by external device\n"));
}

/**
 * Output value to an IO register
 */
void cli_out(int argc, char* argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: out <addr> <value> [value...]\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0xff;
    for (uint8_t i = 2; i < argc; i++) {
        uint8_t value = strtoul(argv[i], NULL, 16) & 0xff;
        io_out(addr, value);
    }
}

/**
 * Input value from an IO register
 */
void cli_in(int argc, char* argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: in <addr>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0xff;
    printf_P(PSTR("Read %02x from %02x\n"), io_in(addr), addr);
}

/**
 * Report the status of the TMS9918A
 */
#ifdef TMS_BASE
void cli_tmsreg(int argc, char* argv[])
{
    tms_report();
}
#endif