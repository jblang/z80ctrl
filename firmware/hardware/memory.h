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

typedef void (*pagefunc_t)(uint8_t, uint8_t, void*);
void mem_read_page(uint8_t start, uint8_t end, void *buf);
void mem_write_page(uint8_t start, uint8_t end, void *buf);
void mem_write_page_P(uint8_t start, uint8_t end, void *buf);
void mem_iterate(uint16_t start, uint16_t end, pagefunc_t dopage, void *buf);
void mem_iterate_banked(uint32_t start, uint32_t end, pagefunc_t dopage, void *buf);

#define mem_read(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_read_page, (buf));
#define mem_write(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_write_page, (buf));
#define mem_write_P(addr, buf, len) mem_iterate((addr), ((addr) + ((len) - 1)), mem_write_page_P, (void *)(buf));

#define mem_read_banked(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_read_page, (buf));
#define mem_write_banked(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_write_page, (buf));
#define mem_write_banked_P(addr, buf, len) mem_iterate_banked((addr), ((addr) + ((len) - 1)), mem_write_page_P, (void *)(buf));
#endif