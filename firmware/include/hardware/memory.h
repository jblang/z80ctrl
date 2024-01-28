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
 * @file memory.h memory-related functions
 */

#ifndef HARDWARE_MEMORY_H
#define HARDWARE_MEMORY_H
#include <stdint.h>

#if defined(BANK_PORT) || defined(BANK_BASE)
void mem_bank(uint8_t bank, uint8_t page);
void mem_bank_addr(uint32_t addr);
#else
#define mem_bank(bank, page)
#define mem_bank_addr(addr)
#endif

void mem_read(uint32_t addr, void* buf, uint32_t len);
void mem_write(uint32_t addr, const void* buf, uint32_t len);
void mem_write_P(uint32_t addr, const void* buf, uint32_t len);

void mem_read_banked(uint32_t addr, void* buf, uint32_t len);
void mem_write_banked(uint32_t addr, const void* buf, uint32_t len);
void mem_write_banked_P(uint32_t addr, const void* buf, uint32_t len);

typedef void (*mem_readfunc_t)(uint32_t, void*, uint32_t);
typedef void (*mem_writefunc_t)(uint32_t, const void*, uint32_t);
uint32_t mem_loadbin(mem_writefunc_t mem_writefunc, char* filename, int32_t start, uint32_t offset, uint32_t len);
void mem_savebin(mem_readfunc_t mem_readfunc, char* filename, uint32_t start, uint32_t len);

#endif