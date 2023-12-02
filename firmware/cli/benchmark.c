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
 * @file benchmark.c bench command
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "fatfs/ff.h"

#include "hardware/bus.h"
#include "hardware/memory.h"
#include "hardware/timer.h"
#include "hardware/uart.h"

#include "util/pgmspace.h"

/**
 * Run a benchmark
 */
//#ifdef BENCHMARK
void cli_bench(int argc, char *argv[])
{
    uint32_t count = 1;
    if (argc == 2)
        count = strtoul(argv[1], NULL, 16);
    for (uint32_t i = 0; i < count; i++) {
        uint8_t sreg = SREG;
        cli();
        config_timer(1, CLKDIV1);

        TCNT1 = 0;
        uint8_t rd = GET_RD;
        uint16_t rdt = TCNT1;

        TCNT1 = 0;
        uint8_t m1 = GET_M1;
        uint16_t m1t = TCNT1;

        TCNT1 = 0;
        uint8_t addrlo = GET_ADDRLO;
        uint16_t addrlot = TCNT1;

        TCNT1 = 0;
        uint8_t addrhi = GET_ADDRHI;
        uint16_t addrhit = TCNT1;

        TCNT1 = 0;
        bus_stat bus = bus_status();
        uint16_t bust = TCNT1;

        TCNT1 = 0;
        bus_stat busfast = bus_status_fast();
        uint16_t busfastt = TCNT1;

        TCNT1 = 0;
        bus_release();
        uint16_t busreleaset = TCNT1;

        TCNT1 = 0;
        bus_request();
        uint16_t busrequestt = TCNT1;

        uint8_t buf[1024];

        TCNT1 = 0;
        mem_read_banked(0, buf, 1024);
        uint16_t memreadbare1kt = TCNT1;

        TCNT1 = 0;
        mem_read_banked(0, buf, 32);
        uint16_t memreadbare32t = TCNT1;

        TCNT1 = 0;
        mem_read_banked(0xf0, buf, 32);
        uint16_t memreadbare32ct = TCNT1;

        TCNT1 = 0;
        mem_read_banked(0, buf, 256);
        uint16_t memreadt = TCNT1;

        FIL fp;
        UINT br;
        TCNT1 = 0;
        f_open(&fp, "/cpm2.dsk", FA_READ);
        uint16_t fopent = TCNT1;

        TCNT1 = 0;
        f_read(&fp, buf, 256, &br);
        uint16_t freadt = TCNT1;

        TCNT1 = 0;
        f_lseek(&fp, 4096);
        uint16_t fseekt = TCNT1;

        TCNT1 = 0;
        f_close(&fp);
        uint16_t fcloset = TCNT1;

        config_timer(1, CLKOFF);
        SREG = sreg;

        printf_P(PSTR("Operation Times:\n"));
        printf_P(PSTR("\tget_rd %d us\n"), TCNT_TO_US(rdt, F_CPU));
        printf_P(PSTR("\tget_m1 %d us\n"), TCNT_TO_US(m1t, F_CPU));
        printf_P(PSTR("\tget_addrlo %d us\n"), TCNT_TO_US(addrlot, F_CPU));
        printf_P(PSTR("\tget_addrhi %d us\n"), TCNT_TO_US(addrhit, F_CPU));
        printf_P(PSTR("\tbus_status %d us\n"), TCNT_TO_US(bust, F_CPU));
        printf_P(PSTR("\tbus_status_fast %d us\n"), TCNT_TO_US(busfastt, F_CPU));
        printf_P(PSTR("\tbus_request %d us\n"), TCNT_TO_US(busrequestt, F_CPU));
        printf_P(PSTR("\tbus_release %d us\n"), TCNT_TO_US(busreleaset, F_CPU));
        printf_P(PSTR("\tmem_read_banked %d us (1KB)\n"), TCNT_TO_US(memreadbare1kt, F_CPU));
        printf_P(PSTR("\tmem_read_banked %d us (32 bytes)\n"), TCNT_TO_US(memreadbare32t, F_CPU));
        printf_P(PSTR("\tmem_read_banked %d us (32 bytes, cross-page)\n"), TCNT_TO_US(memreadbare32ct, F_CPU));
        printf_P(PSTR("\tmem_read_banked %d us\n"), TCNT_TO_US(memreadt, F_CPU));
        printf_P(PSTR("\tf_open %d us\n"), TCNT_TO_US(fopent, F_CPU));
        printf_P(PSTR("\tf_read %d us\n"), TCNT_TO_US(freadt, F_CPU));
        printf_P(PSTR("\tf_lseek %d us\n"), TCNT_TO_US(fseekt, F_CPU));
        printf_P(PSTR("\tf_close %d us\n\n"), TCNT_TO_US(fcloset, F_CPU));

        printf_P(PSTR("Data Structure Sizes:\n"));
        printf_P(PSTR("\tFIL: %d bytes\n"), sizeof(FIL));
        printf_P(PSTR("\tDIR: %d bytes\n"), sizeof(DIR));
        printf_P(PSTR("\tFILINFO: %d bytes\n"), sizeof(FILINFO));
        printf_P(PSTR("\tFSIZE_t: %d bytes\n"), sizeof(FSIZE_t));
        uart_flush();
    }
}
//#endif