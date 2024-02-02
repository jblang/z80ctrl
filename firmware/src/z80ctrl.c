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

#include "z80ctrl.h"
#include "io.h"
#include "term.h"
#include "ffwrap.h"
#include "harvard.h"
#ifdef TMS_BASE
#include "tms9918.h"
#endif
#ifdef SN76489_PORT
#include "sn76489.h"
#endif

const char z80ctrl_banner[] PROGMEM =
    "\n     ___   ___       _        _ "
    "\n ___( _ ) / _ \\  ___| |_ _ __| |"
    "\n|_  / _ \\| | | |/ __| __| '__| |"
    "\n / / (_) | |_| | (__| |_| |  | |"
    "\n/___\\___/ \\___/ \\___|\\__|_|  |_|"
    "\nby J.B. Langston "
#ifdef GITVERSION
    "(" GITVERSION ")"
#endif
    "\n";

const char z80ctrl_copyright[] PROGMEM =
    "\nz80ctrl (https://github.com/jblang/z80ctrl)"
    "\nCopyright (c) 2018-2023 J.B. Langston"
    "\nMIT License"
    "\n"
    "\nFatFS (http://elm-chan.org/fsw/ff/)"
    "\nCopyright (C) 2022, ChaN, all right reserved"
    "\nFatFS License"
    "\n"
    "\nAltair Disk Emulation (https://github.com/open-simh/simh)"
    "\nCopyright (c) 2018-2023, J.B. Langston"
    "\nCopyright (c) 2002-2023, Peter Schorn"
    "\nCopyright (c) 1997-2010, Charles E. Owen"
    "\nMIT License"
    "\n"
    "\nX/Ymodem Protocol (https://www.menie.org/georges/embedded/#xmodem)"
    "\nCopyright 2001-2010 Georges Menie"
    "\nCopyright 2018-2023 J.B. Langston"
    "\n3-clause BSD License"
    "\n"
    "\nFull licenses at https://github.com/jblang/z80ctrl/blob/master/LICENSE.md";

/**
 * z80ctrl entry point
 */
int main(void)
{
    term_init();
    printf_P(z80ctrl_banner);
    bus_init();
    iorq_init();
#ifdef TMS_BASE
    tms_init(TMS_TEXT);
#endif
#ifdef SN76489_PORT
    sn76489_mute();
#endif
    ffw_init();
    cli_start();
}