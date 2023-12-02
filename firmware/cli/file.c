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
 * @file file.c file-related commands
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fatfs/ff.h"

#include "hardware/memory.h"

#include "util/ffwrap.h"
#include "util/ihex.h"
#include "util/pgmspace.h"
#include "util/xmodem.h"

#include "impl.h"

/**
 * Change default directory
 */
void cli_chdir(int argc, char *argv[])
{
 	FRESULT fr;

    if (argc < 2) {
        printf_P(PSTR("usage: cd <directory>\n"));
        return;
    }

    fr = f_chdir(argv[1]);
    if (fr)
        printf_P(PSTR("error changing directory: %S\n"), ffw_error(fr));
}

/**
 * Create a directory
 */
void cli_mkdir(int argc, char *argv[])
{
 	FRESULT fr;

    if (argc < 2) {
        printf_P(PSTR("usage: mkdir <dir>\n"));
        return;
    }

    for (uint8_t i = 1; i < argc; i++) {
        if (fr = f_mkdir(argv[i]))
            printf_P(PSTR("error making directory %s: %S\n"), argv[i], ffw_error(fr));
    }
}

/**
 * Show a directory of files on the SD Card
 */
void cli_dir(int argc, char *argv[])
{
 	FRESULT fr;
    FILINFO finfo;
    DIR dir;
	UINT cnt;
    char *dirname;
    char *glob;
    char buf[14];

    if (argc == 1) {
        dirname = ".";
        glob = NULL;
    } else {
        dirname = argv[1];
        glob = ffw_splitpath(dirname);
    }

    if (glob == NULL) {
        fr = f_opendir(&dir, dirname);
        if (fr == FR_NO_PATH || fr == FR_INVALID_NAME) {
            glob = dirname;
            dirname = ".";
        } else if (fr != FR_OK) {
            printf_P(PSTR("error opening directory: %S\n"), ffw_error(fr));
            return;
        }
    }

    if (glob == NULL) {
        fr = f_readdir(&dir, &finfo);
    } else {
        fr = f_findfirst(&dir, &finfo, dirname, glob);
    }

    uint8_t maxcnt = screenwidth / 13;

    cnt = 0;
    for(;;) {
        if (fr != FR_OK) {
            printf_P(PSTR("error reading directory: %S\n"), ffw_error(fr));
            break;
        }
        if ((cnt % maxcnt == 0 && cnt != 0) || !finfo.fname[0])
            printf_P(PSTR("\n"));
        if (!finfo.fname[0]) 
            break;
        strcpy(buf, finfo.fname);
        if (finfo.fattrib & AM_DIR)
            strcat(buf, "/");
        printf_P(PSTR("%-13s"), buf);
        cnt++;
        if (glob == NULL)
            fr = f_readdir(&dir, &finfo);
        else
            fr = f_findnext(&dir, &finfo);
    }
    if (cnt)
        printf_P(PSTR("%u item(s)\n"), cnt);
    else
        printf_P(PSTR("file not found\n"));
    
    f_closedir(&dir);
}


void cli_copy(int argc, char *argv[])
{
    if (argc < 3) {
        printf_P(PSTR("usage: %s <src>... <dest>\n"), argv[0]);
        return;
    }
    ffw_iterate(ffw_copy, argc-2, &argv[1], argv[argc-1]);
}

void cli_ren(int argc, char *argv[])
{
    if (argc < 3) {
        printf_P(PSTR("usage: %s <src>... <dest>\n"), argv[0]);
        return;
    }
    ffw_iterate(ffw_rename, argc-2, &argv[1], argv[argc-1]);
}

void cli_del(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <file>...\n"), argv[0]);
        return;
    }
    ffw_iterate(ffw_delete, argc-1, &argv[1], NULL);
}

/**
 * Load an Intel HEX file from disk or stdin
 */
void cli_loadhex(int argc, char *argv[])
{
    FIL fil;
    FILE file;
    FRESULT fr;
    ihex_res result;
    if (argc < 2) {
        printf_P(PSTR("loading from console; enter blank line to cancel\n"));
        result = load_ihex(stdin);
    } else {
        printf_P(PSTR("loading from %s\n"), argv[1]);
        if ((fr = ffw_open(&fil, &file, argv[1], FA_READ)) != FR_OK)
            return;
        result = load_ihex(&file);
        ffw_close(&fil);
    }
    printf_P(PSTR("loaded %d bytes total from %04x-%04x"), result.total, result.min, result.max);
    if (result.errors > 0)
        printf_P(PSTR(" with %d errors"), result.errors);
    printf_P(PSTR("\n"));
}

/**
 * Save an Intel HEX file to disk or stdout
 */
void cli_savehex(int argc, char *argv[])
{
    FRESULT fr;
    FILE file;
    FIL fil;
    if (argc < 3) {
        printf_P(PSTR("usage: savehex <start> <end> [file]\n"));
        return;
    }
    uint32_t start = strtoul(argv[1], NULL, 16) & 0xfffff;
    uint32_t end = strtoul(argv[2], NULL, 16) & 0xfffff;
    if (argc == 4) {
        if ((fr = ffw_open(&fil, &file, argv[3], FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK)
            return;
        if (save_ihex(start, end, &file) == EOF)
            printf_P(PSTR("error writing file"));
        ffw_close(&fil);
    } else {
        if (save_ihex(start, end, stdout) == EOF)
            printf_P(PSTR("error writing file"));
    }
}

/**
 * Load a binary file to specifie address from disk with optional offset and length
 */
void cli_loadbin(int argc, char *argv[])
{
    if (argc < 3) {
        printf_P(PSTR("usage: %s <start addr> <filename> [offset] [length]\n"), argv[0]);
        return;
    }
    uint8_t dest;
    if (strcmp_P(argv[0], PSTR("flash")) == 0)
        dest = FLASH;
    else if (strcmp_P(argv[0], PSTR("tmslbin")) == 0)
        dest = TMS;
    else
        dest = MEM;    
    uint32_t start = strtoul(argv[1], NULL, 16);
    char *filename = argv[2];
    uint32_t offset = 0;
    uint32_t len = 0;
    if (argc >= 4)
        offset = strtoul(argv[3], NULL, 16);
    if (argc >= 5)
        len = strtoul(argv[4], NULL, 16);
    ffw_loadbin(filename, dest, start, offset, len);
}

/**
 * Save an region of memory to a binary file on disk
 */
void cli_savebin(int argc, char *argv[])
{
    FRESULT fr;
    FILE file;
    FIL fil;
    UINT bw;
    uint16_t len = 256;
    uint8_t buf[256];
    if (argc < 4) {
        printf_P(PSTR("usage: savebin <start> <end> [file]\n"));
        return;
    }
    uint32_t start = strtoul(argv[1], NULL, 16) & 0xfffff;
    uint32_t end = strtoul(argv[2], NULL, 16) & 0xfffff;
    if ((fr = ffw_open(&fil, NULL, argv[3], FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        while (start <= end) {
            if (end - start + 1 < len)
                len = end - start + 1;
            mem_read_banked(start, buf, len);
            if ((fr = ffw_write(&fil, buf, len, &bw)) != FR_OK)
                break;
            start += len;
        }
        ffw_close(&fil);
    }        
}

/**
 * Receive a file via xmodem
 */
void cli_xmrx(int argc, char *argv[])
{
    printf_P(PSTR("waiting for transfer to begin; press ^X twice to cancel\n"));
    xm_receive(argc - 1, &argv[1]);
}

/**
 * Transmit a file via xmodem
 */
void cli_xmtx(int argc, char *argv[])
{
    FIL fil;
    FILE file;
    FRESULT fr;
    UINT br;
    if (argc < 2) {
        printf_P(PSTR("usage: %s <file>\n"), argv[0]);
        return;
    }
    if ((fr = ffw_open(&fil, NULL, argv[1], FA_READ)) == FR_OK) {
        printf_P(PSTR("beginning transmission; press ^X twice to cancel\n"));
        xm_transmit(&fil);
        ffw_close(&fil);
    }
}

/**
 * Execute the commands in a file
 */
void cli_exec(char *filename)
{
    FIL fil;
    FILE file;
    FRESULT fr;
    char buf[MAXBUF];
    if (strcmp_P(filename, PSTR(AUTOEXEC)) == 0 && f_stat(filename, NULL) == FR_NO_FILE)
        return;
    if ((fr = ffw_open(&fil, &file, filename, FA_READ)) == FR_OK) {
        while (fgets(buf, sizeof buf - 1, &file) != NULL && strlen(buf) != 0) {
            printf_P(PSTR("%s>%s"), filename, buf);
            cli_dispatch(buf);
        }
        ffw_close(&fil);
    }
}

/**
 * Submit the commands in a batch file
 */
void cli_do(int argc, char *argv[]) 
{
    if (argc < 2) {
        printf_P(PSTR("usage: do <filename>\n"));
        return;
    }
    cli_exec(argv[1]);
}