/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
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
 * @file cli.c Monitor command-line interface
 */

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "ff.h"
#include "bus.h"
#include "iorq.h"
#include "z80.h"
#include "ihex.h"
#include "util.h"
#include "disasm.h"
#include "diskemu.h"
#include "bdosemu.h"
#include "sioemu.h"
#include "diskio.h"
#include "uart.h"
#include "xmodem.h"
#include "filedma.h"
#ifdef USE_RTC
#include "rtc.h"
#endif
#ifdef TMS_BASE
#include "tms.h"
#endif
#ifdef SST_FLASH
#include "flash.h"
#endif
#include "ffwrap.h"

/**
 * SD card filesystem
 */
FATFS fs;

/**
 * UART stdio stream
 */
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

uint8_t screenwidth = 80;
uint8_t screenheight = 24;

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
        if ((fr = file_open(&fil, &file, argv[1], FA_READ)) != FR_OK)
            return;
        result = load_ihex(&file);
        file_close(&fil);
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
        if ((fr = file_open(&fil, &file, argv[3], FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK)
            return;
        if (save_ihex(start, end, &file) == EOF)
            printf_P(PSTR("error writing file"));
        file_close(&fil);
    } else {
        if (save_ihex(start, end, stdout) == EOF)
            printf_P(PSTR("error writing file"));
    }
}

#define MEM 0
#define TMS 1
#define FLASH 2

uint32_t loadbin(char *filename, uint8_t dest, int32_t start, uint32_t offset, uint32_t len)
{
    FIL fil;
    FRESULT fr;
    UINT br;
    uint8_t buf[512];
    uint16_t load = start;
    if (len == 0)
        len = 0x100000;
    if ((fr = file_open(&fil, NULL, filename, FA_READ)) != FR_OK)
        return -1;
    if ((fr = file_seek(&fil, offset)) == FR_OK) {
        if (start < 0) {
            // if starting address is not specified, get it from
            // the first two bytes of file a la C64 PRG files
            if (fr = file_read(&fil, &load, 2, &br) != FR_OK) {
                file_close(&fil);
                return 0;
            }
            start = load;
        }
        while ((fr = file_read(&fil, buf, sizeof buf, &br)) == FR_OK) {
            if (br > len)
                br = len;
            if (dest == MEM && br > 0) {
                mem_write_banked(start, buf, br);
            }
#ifdef SST_FLASH
            else if (dest == FLASH) {
                flash_write(start, buf, br);
            }
#endif
#ifdef TMS_BASE
            else if (dest == TMS) {
                tms_write(start, buf, br);
            }
#endif
            if (br < sizeof buf)
                break;
            start += br;
            len -= br;
        }
    }
    file_close(&fil);
    return load;
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
    loadbin(filename, dest, start, offset, len);
}


#ifdef SST_FLASH
/**
 * Erase a flash sector or entire chip
 */
void cli_erase(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: erase <addr> | erase all\n"));
        return;
    }
    uint32_t addr;
    if (strcmp_P(argv[1], PSTR("all")) == 0)
        addr = 0x80000;
    else
        addr = strtoul(argv[1], NULL, 16);
    flash_erase(addr);
}
#endif

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
    if ((fr = file_open(&fil, NULL, argv[3], FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        while (start <= end) {
            if (end - start + 1 < len)
                len = end - start + 1;
            mem_read_banked(start, buf, len);
            if ((fr = file_write(&fil, buf, len, &bw)) != FR_OK)
                break;
            start += len;
        }
        file_close(&fil);
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
    if ((fr = file_open(&fil, NULL, argv[1], FA_READ)) == FR_OK) {
        printf_P(PSTR("beginning transmission; press ^X twice to cancel\n"));
        xm_transmit(&fil);
        file_close(&fil);
    }
}

/**
 * Disassemble code from memory
 */
void cli_disasm(int argc, char *argv[])
{
    uint32_t start, end;
    if (argc < 2) {
        printf_P(PSTR("usage: disasm <start> [end]\n"));
        return;
    }
    start = strtoul(argv[1], NULL, 16) & 0xffff;
    if (argc < 3)
        end = start + 0xf;
    else
        end = strtoul(argv[2], NULL, 16) & 0xffff;
    disasm_mem(start, end);
}

/**
 * Generate a hex dump from memory
 */
void cli_dump(int argc, char *argv[])
{
    uint8_t tms = (strcmp_P(argv[0], PSTR("tmsdump")) == 0);
    if (argc < 2) {
        printf_P(PSTR("usage: %s [start] [end]\n"), argv[0]);
    }

    static uint32_t start = 0;
    uint32_t end = 0;
    if (argc >= 2)
        start = strtoul(argv[1], NULL, 16);
    if (argc >= 3)
        end = strtoul(argv[2], NULL, 16);
    
    uint8_t buf[16];

    uint8_t buflen = 16;
    if (screenwidth < 80)
        buflen = 8;

    if (end == 0)
        end = start + (screenheight - 3) * buflen;

    printf_P(PSTR("ADDR   "));
    for (uint8_t j = 0; j < buflen; j++) {
        printf("+%X ", j);
        if (j % 4 == 3 && screenwidth > 41)
            printf_P(PSTR(" "));
    }
    if (screenwidth > 40)
        printf_P(PSTR(" "));
    for (uint8_t j = 0; j < buflen; j++)
        printf_P(PSTR("%X"), j);
    printf_P(PSTR("\n"));

    
    for (uint32_t i = start; i <= end;) {
        printf_P(PSTR("%05lX  "), base_addr + i);
#ifdef TMS_BASE
        if (tms)
            tms_read(i, buf, buflen);
        else
#endif
            mem_read_banked(i, buf, buflen);
        for (uint8_t j = 0; j < buflen; j++) {
            printf_P(PSTR("%02X "), buf[j]);
            if (j % 4 == 3 && screenwidth > 41)
                printf_P(PSTR(" "));
        }
        if (screenwidth > 40)
            printf_P(PSTR(" "));
        for (uint8_t j = 0; j < buflen; j++, i++) {
            if (0x20 <= buf[j] && buf[j] <= 0x7e)
                printf_P(PSTR("%c"), buf[j]);
            else
                printf_P(PSTR("."));
        }
        printf_P(PSTR("\n"));
    }
    start = end + buflen;
}

/**
 * Report the status of the TMS9918A
 */
#ifdef TMS_BASE
void cli_tmsreg(int argc, char *argv[])
{
    tms_report();
}
#endif

/**
 * Run the processor at full speed from an optional starting address
 */
void cli_run(int argc, char *argv[])
{
    if (argc >= 2) {
        uint32_t addr = strtoul(argv[1], NULL, 16) & 0xfffff;
        z80_reset(addr);
    }
    z80_run();
}

/**
 * Reset the processor and optionally set up a reset vector
 */
void cli_reset(int argc, char *argv[])
{
    uint32_t addr = 0;
    if (argc >= 2) {
        addr = strtoul(argv[1], NULL, 16) & 0xfffff;
    }
    z80_reset(addr);
}

/**
 * Run the processor with debugging enabled from an optional starting address
 */
void cli_debug(int argc, char *argv[])
{
    if (argc >= 2) {
        uint32_t addr = strtoul(argv[1], NULL, 16) & 0xfffff;
        z80_reset(addr);
    }
    z80_debug(1);
}

/**
 * Run the processor for a specified number of instructions
 */
void cli_step(int argc, char *argv[])
{
    uint32_t cycles = 1;
    if (argc >= 2)
        cycles = strtoul(argv[1], NULL, 10);            
    z80_debug(cycles);
}

/**
 * Reduce the Z80 clock speed by the specified factor
 */
void cli_clkdiv(int argc, char *argv[])
{
    uint8_t tmp = 0;
    if (argc >= 2)
        tmp = strtoul(argv[1], NULL, 10);
    if (tmp > 1)
        set_clkdiv(tmp);
    else
        printf_P(PSTR("usage: clkdiv <divider>; minimum divider is 2\n"));
    uint16_t freq = F_CPU / get_clkdiv() / 1000;
    printf_P(PSTR("current speed is %u.%03u MHz (clkdiv=%d)\n"), freq/1000, freq-(freq/1000)*1000, get_clkdiv());
}

/**
 * Set screen size
 */
void cli_screen(int argc, char *argv[])
{
    uint8_t tmpwidth = 0, tmpheight = 0;
    if (argc >= 2)
        tmpwidth = strtoul(argv[1], NULL, 10);
    if (argc >= 3)
        tmpheight = strtoul(argv[2], NULL, 10);
    if (tmpwidth >= 40)
        screenwidth = tmpwidth;
    if (tmpheight >= 24)
        screenheight = tmpheight;
        printf_P(PSTR("usage: screen <width> <height>\n"));
    uint16_t freq = F_CPU / get_clkdiv() / 1000;
    printf_P(PSTR("current screen size is %dx%d\n"), screenwidth, screenheight);
}

/**
 * Interrogate or set breakpoint and watch ranges
 */
void cli_breakwatch(int argc, char *argv[])
{
    range *ranges;
    uint8_t type;

    if (strcmp_P(argv[0], PSTR("break")) == 0)
        ranges = breaks;
    else
        ranges = watches;
    
    // If no parameters given, show current status
    if (argc == 1) {
        printf_P(PSTR("%s status:\n"), argv[0]);
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            if (!ENABLED(ranges, i))
                printf_P(PSTR("\t%S\tdisabled\n"), strlookup(debug_names, i));
            else
                printf_P(PSTR("\t%S\t%04x-%04x\n"), strlookup(debug_names, i), ranges[i].start, ranges[i].end);
        }
        printf_P(PSTR("\nusage:\n\t%s <type> [start] [end]\n"), argv[0]);
        printf_P(PSTR("\t%s <type> off to disable type\n"), argv[0]);
        printf_P(PSTR("\t%s off to disable all\n"), argv[0]);
        return;
    }
    if (strcmp_P(argv[1], PSTR("off")) == 0) {
        // turn off all ranges
        for (uint8_t i = 0; i < DEBUGCNT; i++) {
            ranges[i].start = 0xffff;
            ranges[i].end = 0;
        }
        return;
    } else {
        // find the debugging type that the user specified
        for (type = 0; type < DEBUGCNT; type++)
            if (strcmp_P(argv[1], strlookup(debug_names, type)) == 0)
                break;
        if (type == DEBUGCNT) {
            printf_P(PSTR("error: unknown type\n"));
            return;
        }
        if (argc == 2) {
            // no range specified, enable for 0x0000-0xffff
            ranges[type].start = 0;
            ranges[type].end = 0xffff;
        } else if (argc >= 3) {
            if (strcmp_P(argv[2], PSTR("off")) == 0) {
                ranges[type].start = 0xffff;
                ranges[type].end = 0;
            } else {
                // get starting address
                ranges[type].start = strtoul(argv[2], NULL, 16);
                if (argc >= 4)
                    // get ending address if specified
                    ranges[type].end = strtoul(argv[3], NULL, 16);
                else
                    // if no ending address, start and end are the same
                    ranges[type].end = ranges[type].start;
            }
        }
    }
}

/**
 * Enable/disable BDOS debug messages
 */
void cli_bdosdbg(int argc, char *argv[])
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
        printf_P(PSTR("error changing directory: %S\n"), strlookup(fr_text, fr));
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
            printf_P(PSTR("error making directory %s: %S\n"), argv[i], strlookup(fr_text, fr));
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
        glob = file_splitpath(dirname);
    }

    if (glob == NULL) {
        fr = f_opendir(&dir, dirname);
        if (fr == FR_NO_PATH || fr == FR_INVALID_NAME) {
            glob = dirname;
            dirname = ".";
        } else if (fr != FR_OK) {
            printf_P(PSTR("error opening directory: %S\n"), strlookup(fr_text, fr));
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
            printf_P(PSTR("error reading directory: %S\n"), strlookup(fr_text, fr));
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
    file_iterate(file_copy, argc-2, &argv[1], argv[argc-1]);
}

void cli_ren(int argc, char *argv[])
{
    if (argc < 3) {
        printf_P(PSTR("usage: %s <src>... <dest>\n"), argv[0]);
        return;
    }
    file_iterate(file_rename, argc-2, &argv[1], argv[argc-1]);
}

void cli_del(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <file>...\n"), argv[0]);
        return;
    }
    file_iterate(file_delete, argc-1, &argv[1], NULL);
}

/**
 * Fill memory with a specified byte for a specified range
 */
void cli_fill(int argc, char *argv[])
{
    if (argc != 4) {
        printf_P(PSTR("usage: fill <start> <end> <value>\n"));
        return;
    }
    uint8_t tms = (strcmp_P(argv[0], PSTR("tmsfill")) == 0);
    uint32_t start = strtoul(argv[1], NULL, 16) & 0xfffff;
    uint32_t end = strtoul(argv[2], NULL, 16) & 0xfffff;
    uint8_t buf[256];
    if (strcmp_P(argv[3], PSTR("asc")) == 0) {
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = i;
    } else if (strcmp_P(argv[3], PSTR("desc")) == 0) {
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = 255 - i;
    } else {
        uint8_t value = strtoul(argv[3], NULL, 16) & 0xff;
        for (uint16_t i = 0; i < 256; i++)
            buf[i] = value;
    }
    
    for (;;) {
        if (end - start > 256) {
#ifdef TMS_BASE
            if (tms)
                tms_write(start, buf, 256);
            else
#endif
                mem_write_banked(start, buf, 256);
            start += 256;
        } else {
#ifdef TMS_BASE
            if (tms)
                tms_write(start, buf, end - start + 1);
            else
#endif
                mem_write_banked(start, buf, end - start + 1);
            break;
        }
    }
}

/**
 * Poke values into memory
 */
void cli_poke(int argc, char *argv[])
{
    uint8_t value;
    if (argc < 2)
        printf_P(PSTR("usage: poke <start> [value...]\n"));
    uint32_t addr = strtoul(argv[1], NULL, 16);
    if (argc >= 3) {
        for (uint8_t i = 2; i < argc; i++) {
            value = strtoul(argv[i], NULL, 16) & 0xff;
            mem_write_banked(addr, &value, 1);
            addr++;
        }
        return;
    }
    printf_P(PSTR("enter valid hex to replace; blank to leave unchanged; 'x' to exit.\n"));
    for (;;) {
        char buf[16];
        mem_read_banked(addr, &value, 1);
        printf_P(PSTR("%05lX=%02X : "), addr + base_addr, value);
        if (fgets(buf, sizeof buf - 1, stdin) == NULL) {
            break;
        } else {
            char *end;
            value = strtoul(buf, &end, 16);
            if (*buf == '\n' || *buf == '\r' || *buf == '\0') {
                addr++;
                continue;
            } else if (end == buf) {
                break;
            } else {
                mem_write_banked(addr, &value, 1);
                addr++;
            }
        }
    }
    
}

/**
 * Output value to an IO register
 */
void cli_out(int argc, char *argv[])
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
void cli_in(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: in <addr>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0xff;
    printf_P(PSTR("Read %02x from %02x\n"), io_in(addr), addr);
}

void cli_ioxread(int argc, char *argv[])
{
    if (argc != 3){ 
        printf_P(PSTR("usage: ioxread <addr> <reg>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0x7;
    if (addr < 1 || addr > 7) {
        printf_P(PSTR("invalid address. must be between 1 and 7\n"));
        return;
    }
    uint8_t reg = strtoul(argv[2], NULL, 16) & 0x1F;
    uint8_t value = iox_read(addr, reg);
    printf_P(PSTR("Read %02x from io expander %x register %02x\n"), value, addr, reg);
}

void cli_ioxwrite(int argc, char *argv[])
{
    if (argc != 4){ 
        printf_P(PSTR("usage: ioxwrite <addr> <reg> <value>\n"));
        return;
    }
    uint8_t addr = strtoul(argv[1], NULL, 16) & 0x7;
    if (addr < 1 || addr > 7) {
        printf_P(PSTR("invalid address. must be between 1 and 7\n"));
        return;
    }
    uint8_t reg = strtoul(argv[2], NULL, 16) & 0x1F;
    uint8_t value = strtoul(argv[3], NULL, 16);
    iox_write(addr, reg, value);
}

#if defined(BANK_PORT) || defined(BANK_BASE)

/**
 * Set the base address in memory
 */
void cli_base(int argc, char *argv[])
{
    if (argc == 2) {
        if (base_addr & 0xFFFF != 0)
            printf_P(PSTR("base address must be a multiple of 0x10000"));
        else
            base_addr = strtoul(argv[1], NULL, 16) & 0xF0000;
    } else {
        printf_P(PSTR("current base address is %05lX\n"), base_addr);
    }
}
#endif

/**
 * Boot from a disk
 */
void cli_boot(int argc, char *argv[])
{
    if (argc == 2) {
        drive_mount(0, argv[1]);
    }
    if(drive_bootload()) {
        z80_reset(0);
        z80_run();
    }
}

/**
 * Load and run a coleco ROM
 */
void cli_coleco(int argc, char *argv[])
{
    uint8_t prevdiv = get_clkdiv();
    uint8_t prevmask = get_haltmask();
    char *coleco_rom = "coleco.rom";
    char *game_rom;
    if (argc > 2)
        coleco_rom = argv[2];
    if (argc > 1) {
        game_rom = argv[1];
    } else {
        printf_P(PSTR("usage: coleco game.rom [coleco.rom]\n"));
        return;
    }
    if (f_stat(game_rom, NULL) != FR_OK) {
        printf_P(PSTR("file not found: %s\n"), game_rom);
        return;
    } else if (f_stat(coleco_rom, NULL) != FR_OK) {
        printf_P(PSTR("file not found: %s\n"), coleco_rom);
        return;
    } 
    loadbin(coleco_rom, MEM, 0, 0, 0);
    loadbin(game_rom, MEM, 0x8000, 0, 0);
    set_clkdiv(5);
    set_haltmask(RESET);
    z80_reset(0);
    z80_run();
    set_haltmask(prevmask);
    set_clkdiv(prevdiv);
}

/**
 * Display the bus status
 */
void cli_bus(int argc, char *argv[]) 
{
    bus_log(bus_status());
}

/**
 * Mount a disk image
 */
void cli_mount(int argc, char *argv[])
{
    if (argc != 3) {
        printf_P(PSTR("usage: mount <drive #> <filename>\n"));
        return;
    }
    uint8_t drv = strtoul(argv[1], NULL, 10);
    char *filename = argv[2];
    drive_mount(drv, filename);
}

/**
 * Unmount a disk image
 */
void cli_unmount(int argc, char *argv[])
{
    if (argc != 2) {
        printf_P(PSTR("usage: unmount <drive #>\n"));
        return;
    }
    uint8_t drv = strtoul(argv[1], NULL, 10);
    drive_unmount(drv);
}

/**
 * Display or set the date on the RTC
 */
#ifdef USE_RTC
void cli_date(int argc, char *argv[])
{
    struct tm date;
    char datestr[30];
    if (argc < 8) {
        rtc_get_date(&date);
        printf_P(PSTR("The time is %d:%02d:%02d on %d/%d/%02d\n"), date.tm_hour, date.tm_min, date.tm_sec, date.tm_mon, date.tm_mday, date.tm_year);
        printf_P(PSTR("usage: date <yy mm dd wd hh mm ss>\n"));
        return;
    }

    date.tm_year = strtoul(argv[1], NULL, 10);
    date.tm_mon = strtoul(argv[2], NULL, 10);
    date.tm_mday = strtoul(argv[3], NULL, 10);
    date.tm_wday = strtoul(argv[4], NULL, 10);
    date.tm_hour = strtoul(argv[5], NULL, 10);
    date.tm_min = strtoul(argv[6], NULL, 10);
    date.tm_sec = strtoul(argv[7], NULL, 10);
    rtc_set_date(&date);
}
#endif

/**
 * Assign a device to a port
 */
void cli_assign(int argc, char *argv[])
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
 * Attach a virtual UART to a physical one
 */
void cli_attach(int argc, char *argv[])
{
    uint8_t dir;
    uint8_t mode = SIO_UNATTACHED;
    char *filename = NULL;
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
 * Set the baud rate for the UART
 */
void cli_baud(int argc, char *argv[]) {
    if (argc != 3) {
        printf_P(PSTR("usage: baud <uart> <baud>\n"));
        return;
    }
    uint8_t uart = strtoul(argv[1], NULL, 10) & 1;
    uint32_t requested = strtoul(argv[2], NULL, 10);
    uint16_t ubrr = 0;
    uint32_t actual = 0;
    // Find closest possible actual baud rate
    for (;;) {
        actual = (uint32_t)F_CPU / ((uint32_t)16 * ((uint32_t)ubrr + (uint32_t)1));
        if (actual <= requested)
            break;
        ubrr++;
        // stop if we have maxed out UBRR
        if (ubrr == 0)
            break;
    }
    printf_P(PSTR("UART %u: requested: %lu, actual: %lu\n"), uart, requested, actual);
    uart_flush();
    uart_init(uart, ubrr);
}

/**
 * Enable or disable halt
 */
void cli_halt(int argc, char *argv[]) {
    if (argc == 2) {
        if (strcmp_P(argv[1], PSTR("on")) == 0 || strcmp_P(argv[1], PSTR("both")) == 0)
            set_haltmask(RESET | HALT);
        else if (strcmp_P(argv[1], PSTR("off")) == 0 || strcmp_P(argv[1], PSTR("reset")) == 0)
            set_haltmask(RESET);
        else if (strcmp_P(argv[1], PSTR("halt")) == 0)
            set_haltmask(HALT);
        else if (strcmp_P(argv[1], PSTR("none")) == 0)
            set_haltmask(0);
    }
    printf_P(PSTR("halt on reset signal is %S\n"), get_haltmask() & RESET ? PSTR("enabled") : PSTR("disabled"));
    printf_P(PSTR("halt on halt signal is %S\n"), get_haltmask() & HALT ? PSTR("enabled") : PSTR("disabled"));
}

/**
 * Enable or disable halt
 */
void cli_haltkey(int argc, char *argv[]) {
    if (argc == 2) {
        if (strcmp_P(argv[1], PSTR("off")) == 0)
            watch_key = 0;
        else if (argv[1][0] == '^' && argv[1][1] <= '_' && argv[1][1] >= '@')
            watch_key = argv[1][1] - 64;
        else if (argv[1][0] == '^' && argv[1][1] <= 'z' && argv[1][1] >= 'a')
            watch_key = argv[1][1] - 96;
        else
            watch_key = strtoul(argv[1], NULL, 16) & 0xff;
    }
    if (watch_key == 0)
        printf_P(PSTR("halt key is disabled\n"));
    else {
        printf_P(PSTR("halt key is %02x "), watch_key);
        if (watch_key <= 74)
            printf_P(PSTR("(^%c)\n"), watch_key + 64);
        else if (watch_key >= 33 && watch_key <= 126)
            printf_P(PSTR("(%c)\n"), watch_key);
        else
            printf_P(PSTR("\n"));
    }
}

/**
 * Clear the screen
 */
void cli_cls(int argc, char *argv[])
{
    printf_P(PSTR("\e[0m\e[;H\e[2J"));
}

/**
 * Send an escape sequence
 */
void cli_esc(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <sequence>...\n"), argv[0]);
    }
    for (uint8_t i = 1; i < argc; i++)
        printf_P(PSTR("\e%s"), argv[i]);    // undocumented feature to send escape code
}

/**
 * Send an ascii character
 */
void cli_ascii(int argc, char *argv[])
{
    if (argc < 2) {
        printf_P(PSTR("usage: %s <hex code>...\n"), argv[0]);
    }
    for (uint8_t i = 1; i < argc; i++) {
        char c = strtoul(argv[i], NULL, 16);
        printf_P(PSTR("%c"), c);
    }
}

/**
 * Run a benchmark
 */
#ifdef BENCHMARK
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
#endif

void cli_dispatch(char *buf);

#define WHITESPACE " \t\r\n"
#define MAXBUF 256
#define MAXARGS 256
#define AUTOEXEC "autoexec.z8c"

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
    if ((fr = file_open(&fil, &file, filename, FA_READ)) == FR_OK) {
        while (fgets(buf, sizeof buf - 1, &file) != NULL && strlen(buf) != 0) {
            printf_P(PSTR("%s>%s"), filename, buf);
            cli_dispatch(buf);
        }
        file_close(&fil);
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


/**
 * Lookup table of monitor command names
 */
const char cli_cmd_names[] PROGMEM = 
    "ascii\0"
    "assign\0"
    "attach\0"
#if defined(BANK_PORT) || defined(BANK_BASE)
    "base\0"
#endif
    "baud\0"
    "bdosdbg\0"
#ifdef BENCHMARK
    "bench\0"
#endif
    "boot\0"
    "bus\0"
    "break\0"
    "c\0"
    "cd\0"
    "chdir\0"
    "clkdiv\0"
    "cls\0"
    "coleco\0"
    "copy\0"
    "cp\0"
#ifdef USE_RTC
    "date\0"
#endif
    "debug\0"
    "del\0"
    "dir\0"
    "disasm\0"
    "do\0"
    "dump\0"
    "d\0"
    "era\0"
#ifdef SST_FLASH
    "erase\0"
#endif
    "esc\0"
    "fill\0"
#ifdef SST_FLASH
    "flash\0"
#endif
    "halt\0"
    "haltkey\0"
    "help\0"
    "in\0"
    "ioxrd\0"
    "ioxwr\0"
    "loadbin\0"
    "loadhex\0"
    "ls\0"
    "md\0"
    "mkdir\0"
    "mount\0"
    "move\0"
    "mv\0"
    "out\0"
    "poke\0"
    "rd\0"
    "ren\0"
    "rm\0"
    "rmdir\0"
    "run\0"
    "reset\0"
    "savebin\0"
    "savehex\0"
    "screen\0"
    "s\0"
    "step\0"
#ifdef TMS_BASE
    "tmsreg\0"
    "tmsdump\0"
    "tmsfill\0"
    "tmslbin\0"
#endif
    "unmount\0"
    "watch\0"
    "xmrx\0"
    "xmtx";

/**
 * Lookup table of help text for monitor commands
 */
const char cli_cmd_help[] PROGMEM =
    "\0"                                            // ascii
    "assign a device to a port\0"                   // assign
    "attach virtual uart\0"                         // attach
#if defined(BANK_PORT) || defined(BANK_BASE)
    "set the base memory address\0"                 // base
#endif
    "configure UART baud rate\0"                    // baud
    "\0"                                            // bdosdbg
#ifdef BENCHMARK
    "\0"                                            // bench
#endif
    "boot from specified disk image\0"              // boot
    "display low-level bus status\0"                // bus
    "set breakpoints\0"                             // break
    "\0"                                            // c
    "change directory\0"                            // cd
    "\0"                                            // chdir
    "set Z80 clock divider\0"                       // clkdiv
    "clear screen\0"                                // cls
    "run a colecovision game\0"                       // coleco
    "copy a file (alias cp)\0"                      // copy
    "\0"                                            // cp
#ifdef USE_RTC
    "display or set the date on the rtc\0"          // date
#endif
    "debug code at address (alias c)\0"             // debug
    "erase a file (alias era, rm)\0"                // del
    "shows directory listing in long format\0"      // dir
    "disassembles memory location\0"                // disasm
    "execute a batch file\0"                        // do
    "dump memory in hex and ascii (alias d)\0"      // dump
    "\0"                                            // d
    "\0"                                            // era
#ifdef SST_FLASH
    "erase flash ROM\0"                             // erase
#endif
    "\0"                                            // esc
    "fill memory with byte\0"                       // fill
#ifdef SST_FLASH
    "flash file to ROM\0"                           // flash
#endif
    "enable or disable halt\0"                      // halt
    "enable halt via keyboard shortcut\0"           // haltkey
    "list available commands\0"                     // help
    "read a value from a port\0"                    // in
    "\0"                                            // ioxread
    "\0"                                            // ioxwrite
    "load binary file to memory\0"                  // loadbin
    "load intel hex file to memory\0"               // loadhex
    "shows directory listing in wide format\0"      // ls
    "\0"                                            // md
    "create a subdirectory (alias md)\0"            // mkdir
    "mount a disk image\0"                          // mount
    "\0"                                            // move
    "\0"                                            // mv
    "write a value to a port\0"                     // out
    "poke values into memory\0"                     // poke
    "\0"                                            // rd
    "rename/move a file or directory (alias mv)\0"  // ren
    "\0"                                            // rm
    "\0"                                            // rmdir
    "execute code at address\0"                     // run
    "\0"                                            // reset
    "save binary file from memory\0"                // savebin
    "save intel hex file from memory\0"             // savehex
    "set screen size\0"                             // screen
    "\0"                                            // s
    "step processor N cycles (alias s)\0"           // step
#ifdef TMS_BASE
    "report tms registers\0"                        // tmsreg
    "dump tms memory in hex and ascii\0"            // tmsdump
    "fill tms memory with byte\0"                   // tmsfill
    "load binary file to tms memory\0"              // tmslbin
#endif
    "unmount a disk image\0"                        // unmount
    "set watch points\0"                            // watch
    "receive a file via xmodem\0"                   // xmrx
    "send a file via xmodem";                       // xmtx

void cli_help(int argc, char *argv[]);

/**
 * Lookup table of function pointers for monitor commands
 */
void * const cli_cmd_functions[] PROGMEM = {
    &cli_ascii,
    &cli_assign,
    &cli_attach,
#if defined(BANK_PORT) || defined(BANK_BASE)
    &cli_base,
#endif
    &cli_baud,
    &cli_bdosdbg,
#ifdef BENCHMARK
    &cli_bench,
#endif
    &cli_boot,
    &cli_bus,
    &cli_breakwatch,
    &cli_debug,     // c
    &cli_chdir,     // cd
    &cli_chdir,
    &cli_clkdiv,
    &cli_cls,
    &cli_coleco,
    &cli_copy,       // copy
    &cli_copy,       // cp
#ifdef USE_RTC
    &cli_date,
#endif
    &cli_debug,
    &cli_del,       // del
    &cli_dir,
    &cli_disasm,
    &cli_do,
    &cli_dump,
    &cli_dump,      // d
    &cli_del,       // era
#ifdef SST_FLASH
    &cli_erase,
#endif
    &cli_esc,       // esc
    &cli_fill,
#ifdef SST_FLASH
    &cli_loadbin,   // flash
#endif
    &cli_halt,
    &cli_haltkey,
    &cli_help,
    &cli_in,
    &cli_ioxread,
    &cli_ioxwrite,
    &cli_loadbin,
    &cli_loadhex,
    &cli_dir,       // ls
    &cli_mkdir,     // md
    &cli_mkdir,
    &cli_mount,
    &cli_ren,        // move
    &cli_ren,        // mv
    &cli_out,
    &cli_poke,
    &cli_del,       // rd
    &cli_ren,
    &cli_del,       // rm
    &cli_del,       // rmdir
    &cli_run,
    &cli_reset,
    &cli_savebin,
    &cli_savehex,
    &cli_screen,
    &cli_step,      // s
    &cli_step,
#ifdef TMS_BASE
    &cli_tmsreg,    // tmsreg
    &cli_dump,      // tmsdump
    &cli_fill,      // tmsfill
    &cli_loadbin,   // tmslbin
#endif
    &cli_unmount,
    &cli_breakwatch,
    &cli_xmrx,
    &cli_xmtx
};

#define NUM_CMDS (sizeof(cli_cmd_functions)/sizeof(void *))

/**
 * List available commands with help text
 */
void cli_help(int argc, char *argv[])
{
    int i;
    printf_P(PSTR("available commands:\n"));
    for (i = 0; i < NUM_CMDS; i++) {
        if (strlen_P(strlookup(cli_cmd_help, i)))
            printf_P(PSTR("%S\t%S\n"), strlookup(cli_cmd_names, i), strlookup(cli_cmd_help, i));
    }
}

#define endswith(str, suf) (strcmp((str) + strlen(str) - strlen(suf), (suf)) == 0)
#define endswith_P(str, suf) (strcmp_P((str) + strlen(str) - strlen_P(suf), (suf)) == 0)

/**
 * Run a CP/M .COM file
 */
uint8_t cli_runcom(int argc, char *argv[])
{
    char filename[256];
    strcpy(filename, argv[0]);
    if (!endswith_P(filename, PSTR(".com")))
        strcat_P(filename, PSTR(".com"));
    if (f_stat(filename, NULL) == FR_OK) {
        // Look for BDOS runtime file in current directory then root directory
        uint16_t start;
        if (f_stat("bdos.bin", NULL) == FR_OK) {
            start = loadbin("bdos.bin", MEM, -1, 0, 0);
        } else if (f_stat("/bdos.bin", NULL) == FR_OK) {
            start = loadbin("/bdos.bin", MEM, -1, 0, 0);
        } else {
            // Last resort: warn and try to run the program without bdos loaded
            printf_P(PSTR("warning: bdos.bin not found in current or root directory.\n"));
            start = 0x100;
        }
        bdos_init(argc, argv);
        loadbin(filename, MEM, 0x100, 0, 0);
        z80_reset(start);
        z80_run();
        putchar('\n');
        return 1;
    } else {
        return 0;
    }
}

/**
 * Run a .PRG file (binary with start address)
 */
uint8_t cli_runprg(int argc, char *argv[])
{
    char filename[256];
    FILINFO fno;
    strcpy(filename, argv[0]);
    if (!endswith_P(filename, PSTR(".prg")))
        strcat_P(filename, PSTR(".prg"));
    if (f_stat(filename, NULL) == FR_OK) {
        save_cli(argc, argv);
        uint16_t start = loadbin(filename, MEM, -1, 0, 0);
        z80_reset(start);
        z80_run();
        putchar('\n');
        return 1;
    } else {
        return 0;
    }
}

/**
 * Dispatch a single command
 */
void cli_dispatch(char *buf) 
{
    char *cmd;
    char *argv[MAXARGS];
    int argc;
    int i;
    void (*cmd_func)(int, char*[]);
    if ((argv[0] = strtok(buf, WHITESPACE)) == NULL)
        return;
    for (argc = 1; argc < MAXARGS; argc++) {
        if ((argv[argc] = strtok(NULL, WHITESPACE)) == NULL)
            break;
    }
    for (i = 0; i < strlen(argv[0]); i++)
        argv[0][i] = tolower(argv[0][i]);
    for (i = 0; i < NUM_CMDS; i++) {
        if (strcmp_P(argv[0], strlookup(cli_cmd_names, i)) == 0) {
            cmd_func = pgm_read_ptr(&cli_cmd_functions[i]);
            break;
        }
    }
    if (i < NUM_CMDS) {
        cmd_func(argc, argv);
    } else if (!cli_runcom(argc, argv)) {
        if (!cli_runprg(argc, argv)) {
            printf_P(PSTR("unknown command: %s. type help for list.\n"), argv[0]);
        }
    }
}

/**
 * Main command-line loop for monitor
 */
void cli_loop(void) 
{
 	FRESULT fr;
    char buf[MAXBUF];
    printf_P(PSTR(
    "\n      ___   ___       _        _ "
    "\n  ___( _ ) / _ \\  ___| |_ _ __| |"
    "\n |_  / _ \\| | | |/ __| __| '__| |"
    "\n  / / (_) | |_| | (__| |_| |  | |"
    "\n /___\\___/ \\___/ \\___|\\__|_|  |_|"
    "\n by J.B. Langston "
#ifdef GITVERSION
        "(" GITVERSION ")"
#endif
        "\n\ntype help to list available commands\n"));

    for (;;) {
        fr = f_getcwd(buf, MAXBUF);
        if (fr)
            buf[0] = '\0';
        printf_P(PSTR("z80ctrl:%s>"), buf);
        if (fgets(buf, sizeof buf - 1, stdin) != NULL)
            cli_dispatch(buf);
    }
}

/**
 * z80ctrl entry point
 */
int main(void)
{
    FRESULT fr;

    uart_init(0, UBRR115200);
    uart_init(1, UBRR115200);
    stdout = stdin = &uart_str;

    disk_initialize(DRV_MMC);
    if ((fr = f_mount(&fs, "", 1)) != FR_OK)
        printf_P(PSTR("error mounting drive: %S\n"), strlookup(fr_text, fr));

    bus_init();
    iorq_init();
 #ifdef TMS_BASE
    tms_init(TMS_TEXT);
#endif
    cli_exec(AUTOEXEC);
    cli_loop();
}