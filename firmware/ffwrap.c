/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation argvs (the "Software"), 
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
 * @argv ffwrap.c Wrapper functions for FatFS
 */

#include <stdint.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <ff.h>

#include "ffwrap.h"
#include "util.h"

char *file_splitpath(const char *path)
{
    char *fs = strrchr(path, '/');
    char *bs = strrchr(path, '\\');
    if  (fs > bs) {
        *fs = '\0';
        return fs + 1;
    } else if (bs != NULL) {
        *bs = '\0';
        return bs + 1;
    }
    return NULL;
}

/**
 * FatFS wrapper to write a single byte to a file, used by stdio library
 */
int fatfs_putchar(char c, FILE *stream)
{
    FIL *fil = stream->udata;
    if (f_write(fil, &c, 1, NULL) != FR_OK)
        return EOF;
    else
        return 0;
}

/**
 * FatFS wrapper to read a single byte from a file, used by stdio library
 */
int fatfs_getchar(FILE *stream)
{
    FIL *fil = stream->udata;
    char c;

    if (f_read(fil, &c, 1, NULL) == FR_OK)
        return c;
    else
        return EOF;
}

FRESULT file_open(FIL *fil, FILE *file, const TCHAR *filename, BYTE mode)
{
    FRESULT fr;
    if ((fr = f_open(fil, filename, mode)) != FR_OK) {
        printf_P(PSTR("error opening '%s': %S\n"), filename, strlookup(fr_text, fr));
    } else if (file != NULL) {
        fdev_setup_stream(file, fatfs_putchar, fatfs_getchar, _FDEV_SETUP_RW);
        fdev_set_udata(file, fil);
    }
    return fr;
}

FRESULT file_close(FIL *fil)
{
    FRESULT fr;
    if ((fr = f_close(fil)) != FR_OK) {
        printf_P(PSTR("error closing file: %S\n"), strlookup(fr_text, fr));
    }
    return fr;
}

FRESULT file_read(FIL* fil, void* buff, UINT btr, UINT* br)
{
    FRESULT fr;
    if ((fr = f_read(fil, buff, btr, br)) != FR_OK)
        printf_P(PSTR("error reading file: %S\n"), strlookup(fr_text, fr));
    return fr;
}

FRESULT file_write(FIL* fil, void* buff, UINT btw, UINT* bw)
{
    FRESULT fr;
    if ((fr = f_write(fil, buff, btw, bw)) != FR_OK)
        printf_P(PSTR("error writing file: %S\n"), strlookup(fr_text, fr));
    return fr;
}

FRESULT file_seek(FIL* fil, FSIZE_t ofs)
{
    FRESULT fr;
    if ((fr = f_lseek(fil, ofs)) != FR_OK)
        printf_P(PSTR("error seeking file: %S\n"), strlookup(fr_text, fr));
    return fr;
}

FRESULT file_delete(const TCHAR* path, const TCHAR* dummy)
{
    FRESULT fr;
    if ((fr = f_unlink(path)) != FR_OK)
        printf_P(PSTR("error deleting '%s': %S\n"), path, strlookup(fr_text, fr));
    return fr;
}

FRESULT file_rename(const TCHAR* path_old, const TCHAR* path_new)
{
    FRESULT fr;
    if ((fr = f_rename(path_old, path_new)) != FR_OK)
        printf_P(PSTR("error renaming '%s' to '%s': %S\n"), path_old, path_new, strlookup(fr_text, fr));
    return fr;
}

FRESULT file_copy(const TCHAR* path_old, const TCHAR* path_new)
{
    FRESULT fr;
    if ((fr = f_copy(path_old, path_new)) != FR_OK)
        printf_P(PSTR("error copying '%s' to '%s': %S\n"), path_old, path_new, strlookup(fr_text, fr));
    return fr;
}

void file_iterate(operation_t operation, int count, char* file[], char* dest)
{
 	FRESULT fr;
    DIR dir;
    FILINFO fno;
    char srcpath[128], destpath[128];

    uint8_t dest_is_dir = dest != NULL && f_stat(dest, &fno) == FR_OK && fno.fattrib & AM_DIR;    
    for (uint8_t i = 0; i < count; i++) {
        const char *path1 = file[i];
        const char *path2 = file_splitpath(file[i]);
        if (path2 == NULL)
            fr = f_findfirst(&dir, &fno, ".", path1);
        else
            fr = f_findfirst(&dir, &fno, path1, path2);
        
        while (fr == FR_OK && fno.fname[0] != 0) {
            if (path2 != NULL) {
                strcpy(srcpath, path1);
                strcat_P(srcpath, PSTR("/"));
                strcat(srcpath, fno.fname);
            } else {
                strcpy(srcpath, fno.fname);
            }
            if (dest != NULL) {
                strcpy(destpath, dest);
                if (dest_is_dir) {
                    strcat_P(destpath, PSTR("/"));
                    strcat(destpath, fno.fname);
                }
            }
            operation(srcpath, destpath);
            fr = f_findnext(&dir, &fno);
        }
        fr = f_closedir(&dir);
    }
    if (fr != FR_OK)
        printf_P(PSTR("error enumerating files: %S\n"), strlookup(fr_text, fr));
}
