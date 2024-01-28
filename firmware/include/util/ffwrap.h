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
 * @file ffwrap.h Wrapper functions for FatFS
 */

#ifndef UTIL_FFWRAP_H
#define UTIL_FFWRAP_H

#include <stdint.h>
#include <stdio.h>

#include "fatfs/ff.h"

typedef FRESULT (*operation_t)(const TCHAR* path_old, const TCHAR* path_new);

#define MEM 0
#define TMS 1
#define FLASH 2

char* ffw_splitpath(const char* path);
FRESULT ffw_open(FIL* fil, FILE* file, const TCHAR* filename, BYTE mode);
FRESULT ffw_close(FIL* fil);
FRESULT ffw_read(FIL* fil, void* buff, UINT btr, UINT* br);
FRESULT ffw_write(FIL* fil, void* buff, UINT btw, UINT* bw);
FRESULT ffw_seek(FIL* fil, FSIZE_t ofs);
FRESULT ffw_delete(const TCHAR* path, const TCHAR* dummy);
FRESULT ffw_rename(const TCHAR* path_old, const TCHAR* path_new);
FRESULT ffw_copy(const TCHAR* path_old, const TCHAR* path_new);
void ffw_iterate(operation_t operation, int count, char* file[], char* dest);
const char* ffw_error(FRESULT fr);
void ffw_init();

#endif