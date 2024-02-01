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
 * @file memory.c memory-related functions
 */

#include <avr/cpufunc.h>
#include <stdlib.h>

#include "ffwrap.h"
#include "harvard.h"

#include "bus.h"
#include "io.h"
#include "memory.h"

/**
 * Bank in the page at the specified address
 */
#ifdef BANK_PORT
uint8_t mem_banks = 0;

void mem_bank_addr(uint32_t addr)
{
    mem_banks = (addr >> 15) & 0x0e;
    mem_banks |= (mem_banks + 1) << 4;
    io_out(BANK_PORT, mem_banks);
}
#elif defined(BANK_BASE)
#define BANK_ENABLE (BANK_BASE + 4)
uint8_t mem_banks[] = { 0, 0, 0, 0 };
void mem_bank(uint8_t bank, uint8_t addr)
{
    bank &= 3;
    addr &= 0x3f;
    io_out(BANK_ENABLE, 1);
    io_out(BANK_BASE + bank, addr);
    mem_banks[bank] = addr;
}

void mem_bank_addr(uint32_t addr)
{
    uint8_t page = (addr >> 14) & 0x3c;
    for (uint8_t i = 0; i < 4; i++)
        mem_bank(i, page + i);
}
#endif

void mem_read_page(uint8_t start, uint8_t end, void* buf)
{
    if (GET_BUSACK)
        return;
    uint8_t* bytebuf = buf;
    DATA_INPUT;
    RD_LO;
    do {
        SET_ADDRLO(start++);
        _NOP();
        _NOP();
        *bytebuf++ = GET_DATA;
    } while (start <= end && start != 0);
    RD_HI;
}

void mem_write_page(uint8_t start, uint8_t end, void* buf)
{
    if (GET_BUSACK)
        return;
    uint8_t* bytebuf = buf;
    DATA_OUTPUT;
    do {
        SET_ADDRLO(start++);
        SET_DATA(*bytebuf++);
        WR_LO;
        WR_HI;
    } while (start <= end && start != 0);
    DATA_INPUT;
}

void mem_write_page_P(uint8_t start, uint8_t end, void* buf)
{
    if (GET_BUSACK)
        return;
    uint8_t* bytebuf = buf;
    DATA_OUTPUT;
    do {
        SET_ADDRLO(start++);
        SET_DATA(pgm_read_byte(bytebuf++));
        WR_LO;
        WR_HI;
    } while (start <= end && start != 0);
    DATA_INPUT;
}

typedef void (*pagefunc_t)(uint8_t, uint8_t, void*);

void mem_iterate(uint16_t start, uint16_t end, pagefunc_t pagefunc, void* buf)
{
    if (GET_BUSACK)
        return;
    uint8_t starthi = start >> 8;
    uint8_t startlo = start & 0xff;
    uint8_t endhi = end >> 8;
    uint8_t endlo = end & 0xff;
    MREQ_LO;
    SET_ADDRHI(starthi);
    if (starthi == endhi) {
        pagefunc(startlo, endlo, buf);
    } else {
        pagefunc(startlo, 0xff, buf);
        buf += 0x100 - startlo;
        starthi++;
        while (starthi < endhi) {
            SET_ADDRHI(starthi++);
            pagefunc(0, 0xff, buf);
            buf += 0x100;
        }
        SET_ADDRHI(starthi);
        pagefunc(0, endlo, buf);
    }
    MREQ_HI;
}

void mem_iterate_banked(uint32_t start, uint32_t end, pagefunc_t pagefunc, void* buf)
{
    if (GET_BUSACK)
        return;
    start += base_addr;
    end += base_addr;
    mem_bank_addr(start);
    if ((start & 0x10000) == (end & 0x10000)) {
        mem_iterate(start & 0xffff, end & 0xffff, pagefunc, buf);
    } else {
        mem_iterate(start & 0xffff, 0xffff, pagefunc, buf);
        buf += 0x10000 - start;
        mem_bank_addr(end);
        mem_iterate(0, end & 0xffff, pagefunc, buf);
    }
}

void mem_read(uint32_t addr, void* buf, uint32_t len)
{
    return mem_iterate(addr, (addr + (len - 1)), mem_read_page, buf);
}

void mem_write(uint32_t addr, const void* buf, uint32_t len)
{
    return mem_iterate(addr, (addr + (len - 1)), mem_write_page, (void*)buf);
}

void mem_write_P(uint32_t addr, const void* buf, uint32_t len)
{
    return mem_iterate(addr, (addr + (len - 1)), mem_write_page_P, (void*)buf);
}

void mem_read_banked(uint32_t addr, void* buf, uint32_t len)
{
    return mem_iterate_banked(addr, (addr + (len - 1)), mem_read_page, buf);
}

void mem_write_banked(uint32_t addr, const void* buf, uint32_t len)
{
    return mem_iterate_banked(addr, (addr + (len - 1)), mem_write_page, (void*)buf);
}

void mem_write_banked_P(uint32_t addr, const void* buf, uint32_t len)
{
    return mem_iterate_banked(addr, (addr + (len - 1)), mem_write_page_P, (void*)buf);
}

uint32_t mem_loadbin(mem_writefunc_t mem_writefunc, char* filename, int32_t start, uint32_t offset, uint32_t len)
{
    FIL fil;
    FRESULT fr;
    UINT br;
    uint8_t buf[512];
    uint16_t load = start;
    if (len == 0)
        len = 0x100000;
    if ((fr = ffw_open(&fil, NULL, filename, FA_READ)) != FR_OK)
        return -1;
    if ((fr = ffw_seek(&fil, offset)) == FR_OK) {
        if (start < 0) {
            // if starting address is not specified, get it from
            // the first two bytes of file a la C64 PRG files
            if (fr = ffw_read(&fil, &load, 2, &br) != FR_OK) {
                ffw_close(&fil);
                return 0;
            }
            start = load;
        }
        while ((fr = ffw_read(&fil, buf, sizeof buf, &br)) == FR_OK) {
            if (br > len)
                br = len;
            if (br > 0)
                mem_writefunc(start, buf, br);
            if (br < sizeof buf)
                break;
            start += br;
            len -= br;
        }
    }
    ffw_close(&fil);
    return load;
}

/**
 * Save an region of memory to a binary file on disk
 */
void mem_savebin(mem_readfunc_t mem_readfunc, char* filename, uint32_t start, uint32_t len)
{
    FRESULT fr;
    FILE file;
    FIL fil;
    UINT bw;
    uint8_t buf[256];
    uint32_t end = start + len - 1;
    len = sizeof buf;
    if ((fr = ffw_open(&fil, NULL, filename, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        while (start <= end) {
            if (end - start + 1 < len)
                len = end - start + 1;
            mem_readfunc(start, buf, len);
            if ((fr = ffw_write(&fil, buf, len, &bw)) != FR_OK)
                break;
            start += len;
        }
        ffw_close(&fil);
    }
}