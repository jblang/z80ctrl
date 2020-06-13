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
#include "tms.h"

#include <stdint.h>
#include <avr/pgmspace.h>

const uint8_t defcolors[] PROGMEM = {
    0x0, 0x6, 0xc, 0xa, 0x4, 0xd, 0x7, 0xe, 
    0xe, 0x9, 0x3, 0xb, 0x5, 0xd, 0x7, 0xf
};

#define DEFCOLOR(i) (pgm_read_byte(&defcolors[i]))

uint8_t colormap[] = {
    0x0, 0x6, 0xc, 0xa, 0x4, 0xd, 0x7, 0xe, 
    0xe, 0x9, 0x3, 0xb, 0x5, 0xd, 0x7, 0xf
};

int16_t cursorpos;
uint8_t undercursor;

void term_literal(uint8_t c)
{
    undercursor = c;
    tms_write(nametab + cursorpos, &c, 1);
    cursorpos++;
}

void term_scroll(int16_t lines)
{
    uint8_t buf[1000];
    static uint16_t prevpos;
    tms_write(nametab + prevpos, &undercursor, 1);
    if (lines > 0) {
        tms_read(nametab + lines * 40, buf, 960 - lines * 40);
        memset(buf + 960 - lines * 40, 0, lines * 40);
        tms_write(nametab, buf, 960);
    } else if (lines < 0) {
        lines = -lines;
        tms_read(nametab, buf + lines * 40, 960 - lines * 40);
        memset(buf, 0, lines * 40);
        tms_write(nametab, buf, 960);
    }
    prevpos = cursorpos;
}

void term_update()
{
    uint8_t cursor[8];
    int8_t lines = 0;
    if (cursorpos < 0) {
        lines = cursorpos / 40;
        cursorpos = -cursorpos % 40;
    } else if (cursorpos > 959) {
        lines = (cursorpos - 920) / 40;
        cursorpos = 920 + (cursorpos % 40);
    }

    term_scroll(lines);
    tms_read(nametab + cursorpos, &undercursor, 1);
    if (undercursor == 0 || undercursor == 32) {
        tms_fill(nametab + cursorpos, 0xdb, 1);
    } else {
        tms_read(pattab + undercursor * 8, cursor, 8);
        for (uint8_t i = 0; i < 8; i++)
            cursor[i] = ~cursor[i];
        tms_write(pattab + 0xff * 8, cursor, 8);
        tms_fill(nametab + cursorpos, 0xff, 1);
    }
}

void term_delete()
{
    cursorpos--;
    undercursor = 0;
}

void term_home()
{
    cursorpos = 0;
}

void term_cleartext()
{
    tms_fill(nametab, 0, 960);
}

void term_cleargraph()
{
}

void term_cursorup(uint8_t c)
{
    cursorpos -= 40 * c;
}

void term_cursordown(uint8_t c)
{
    cursorpos += 40 * c;
}

void term_cursorleft(uint8_t c)
{
    cursorpos -= c;
}

void term_cursorright(uint8_t c)
{
    cursorpos += c;
}

void term_startline()
{
    cursorpos = (cursorpos / 40) * 40;
}

void term_pos(uint8_t x, uint8_t y)
{
    cursorpos = (y % 24) * 40 + (x % 40);
}

void vdu_color(uint8_t *p)
{
    if (p[1] >= 128)
        textbg = colormap[p[1] & 0xf];
    else
        textfg = colormap[p[1] & 0xf];
    tms_config();
}

void vdu_gcolor(uint8_t *p)
{
    pixelmode = p[1] % 5;
    if (p[2] >= 128)
        pixelbg = colormap[p[2] & 0xf];
    else
        pixelfg = colormap[p[2] & 0xf];
    tms_config();
}

void vdu_lcolor(uint8_t *p)
{
    colormap[p[1] & 0xf] = p[2] & 0xf;
}

void vdu_dcolor(uint8_t *p)
{
    for (uint8_t i = 0; i < 16; i++)
        colormap[i] = DEFCOLOR(i);
}

void vdu_flash(uint8_t *p)
{
    uint8_t save = textbg;
    textbg = ~textbg;
    tms_config();
    _delay_ms(100);
    textbg = save;
    tms_config();
}

void vdu_pos(uint8_t *p)
{
    term_pos(p[1], p[2]);
}

void vdu_home(uint8_t *p)
{
    term_home();
}

void vdu_cleartext(uint8_t *p)
{
    tms_fill(nametab, 0, 960);
}

void vdu_cleargraph(uint8_t *p)
{
}

void vdu_cursorup(uint8_t *p)
{
    term_cursorup(1);
}

void vdu_cursordown(uint8_t *p)
{
    term_cursordown(1);
}

void vdu_cursorleft(uint8_t *p)
{
    term_cursorleft(1);
}

void vdu_cursorright(uint8_t *p)
{
    term_cursorright(1);
}

void vdu_startline(uint8_t *p)
{
    term_startline();
}

void term_mode(uint8_t *p)
{
    switch (p[1]) {
        case 0:
            tms_init(TMS_TEXT);
            break;
        case 1:
            tms_init(TMS_TILE);
            break;
        case 2:
            tms_init(TMS_BITMAP);
            break;
        case 3:
            tms_init(TMS_MULTICOLOR);
            break;
    }
}

void vdu_program(uint8_t *p)
{

}

void vdu_plot(uint8_t *p)
{

}

void vdu_gwindow(uint8_t *p)
{
    
}

void vdu_dwindow(uint8_t *p)
{

}

void vdu_twindow(uint8_t *p)
{

}

void vdu_origin(uint8_t *p)
{

}

void ansi_clearscreen(uint8_t n)
{
    if (n == 0) {
        tms_fill(nametab + cursorpos, 0, 960 - cursorpos);
    } else if (n == 1) {
        tms_fill(nametab, 0, cursorpos);
    } else if (n == 2 || n == 3) {
        tms_fill(nametab, 0, 960);
    }
    undercursor = 0;
}

void ansi_clearline(uint8_t n)
{
    int startline = (cursorpos / 40) * 40;
    int column = cursorpos % 40;
    if (n == 0) {
        tms_fill(nametab + cursorpos, 0, 40 - column);
    } else if (n == 1) {
        tms_fill(nametab + startline, 0, column);
    } else if (n == 2) {
        tms_fill(nametab + startline, 0, 40);
    }
    undercursor = 0;
}

void ansi_color(uint8_t n)
{
    if (30 <= n && n <= 37)
        textfg = DEFCOLOR(n - 30);
    else if (n == 38)
        textfg = 0;
    else if (40 <= n && n <= 47)
        textbg = DEFCOLOR(n - 40);
    else if (n == 48)
        textbg = 0;
    else if (90 <= n && n <= 97)
        textfg = DEFCOLOR(n - 90 + 8);
    else if (100 <= n && n <= 107)
        textbg = DEFCOLOR(n - 100 + 8);
    tms_config();
}


void ansi_insertlines(uint8_t lines)
{
    uint8_t buf[1000];
    uint16_t start = cursorpos / 40;
    if (lines > 24 - start)
        lines = 24 - start;
    start *= 40;
    lines *= 40;
    tms_read(nametab + start, buf + lines, 960 - start - lines);
    buf[lines] = undercursor;
    memset(buf, 0, lines);
    tms_write(nametab + start, buf, 960 - start);
    undercursor = 0;
}

void ansi_deletelines(uint8_t lines)
{
    uint8_t buf[1000];
    uint16_t start = cursorpos / 40;
    if (lines > 24 - start)
        lines = 24 - start;
    start *= 40;
    lines *= 40;
    tms_read(nametab + start + lines, buf, 960 - start - lines);
    memset(buf + 960 - start - lines, 0, lines);
    tms_write(nametab + start, buf, 960 - start);
    undercursor = buf[0];
}

void ansi_command(uint8_t command, uint8_t paramcnt, uint8_t params[])
{
    static uint16_t savepos;
    uint8_t cnt = params[0] == 0 ? 1 : params[0];
    uint8_t cnt2 = params[1] == 0 ? 1 : params[1];
    switch (command) {
        case 'A':
            term_cursorup(cnt);
            break;
        case 'B':
            term_cursordown(cnt);
            break;
        case 'C':
            term_cursorright(cnt);
            break;
        case 'D':
            term_cursorleft(cnt);
            break;
        case 'E':
            term_startline();
            term_cursordown(cnt);
            break;
        case 'F':
            term_startline();
            term_cursorup(cnt);
            break;
        case 'G':
            term_startline();
            term_cursorright(cnt - 1);
        case 'H':
            term_pos(cnt2 - 1, cnt - 1);
            break;
        case 'J':
            ansi_clearscreen(params[0]);
            break;
        case 'K':
            ansi_clearline(params[0]);
            break;
        case 'L':
            ansi_insertlines(cnt);
            break;
        case 'M':
            ansi_deletelines(cnt);
            break;
        case 'S':
            term_scroll(cnt);
            break;
        case 'T':
            term_scroll(-cnt);
            break;
        case 'm':
            for (uint8_t i = 0; i <= paramcnt; i++)
                ansi_color(params[i]);
            break;
        case 's':
            savepos = cursorpos;
            break;
        case 'u':
            cursorpos = savepos;
            break;
    }
}

uint8_t const vdu_length[] PROGMEM = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 0, 1, 9, 8, 5, 0, 1, 4, 4, 0, 2
};

void * const vdu_functions[] PROGMEM = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    vdu_flash,
    vdu_cursorleft,
    vdu_cursorright,
    vdu_cursordown,
    vdu_cursorup,
    vdu_cleartext,
    vdu_startline,
    NULL,
    NULL,
    vdu_cleargraph,
    vdu_color,
    vdu_gcolor,
    vdu_lcolor,
    vdu_dcolor,
    NULL,
    term_mode,
    vdu_program,
    vdu_gwindow,
    vdu_plot,
    vdu_dwindow,
    NULL,
    vdu_twindow,
    vdu_origin,
    vdu_home,
    vdu_pos
};

#define MAXPARAM 10

enum term_mode {
    TERM_NORMAL,
    TERM_COMMAND,
    TERM_DISABLED,
    TERM_ESC,
    TERM_CSI
};

void term_putchar(uint8_t c)
{
    static uint8_t mode = TERM_NORMAL;
    static uint8_t paramidx = 0;
    static uint8_t paramlen = 0;
    static uint8_t parambuf[MAXPARAM];

    if (mode == TERM_DISABLED) {
        if (c == 6)
            mode = TERM_NORMAL;
        return;
    }

    if (GET_BUSACK)
        bus_master();

    if (mode == TERM_NORMAL) {
        if (c == 21) {
            mode = TERM_DISABLED;
        } else if (c == '\e') {
            mode = TERM_ESC;
        } else if (c == 0x7f) {
            term_delete();
        } else if (c < 32) {
            paramidx = 0;
            paramlen = pgm_read_byte(&vdu_length[c]);
            mode = TERM_COMMAND;
        } else {
            term_literal(c);
        }
    } else if (mode == TERM_ESC) {
        if (c == '[') {
            memset(parambuf, 0, MAXPARAM);
            paramidx = 0;
            mode = TERM_CSI;
        } else {
            if (c == 'c') {   // reset terminal
                tms_init(TMS_TEXT);
            } else {
                term_literal(c);
            }
            mode = TERM_NORMAL;
        }
    } else if (mode == TERM_CSI) {
        if ('0' <= c && c <= '9') {
            parambuf[paramidx] = parambuf[paramidx] * 10 + c - '0';
        } else if ((c == ':' || c == ';') && paramidx < MAXPARAM-1) {
            paramidx++;
        } else {
            ansi_command(c, paramidx, parambuf);
            mode = TERM_NORMAL;
        }
    }

    if (mode == TERM_COMMAND) {
        parambuf[paramidx++] = c;
        if (paramidx > paramlen) {
            void (*vdu_func)(uint8_t[]) = pgm_read_ptr(&vdu_functions[parambuf[0]]);
            if (vdu_func)
                vdu_func(parambuf);
            mode = TERM_NORMAL;
        }
    }

    term_update();
}