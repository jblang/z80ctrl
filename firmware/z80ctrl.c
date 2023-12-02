#include "util/pgmspace.h"

#include "cli/cli.h"

#include "hardware/hardware.h"
#include "hardware/term.h"

#include "util/ffwrap.h"

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
    "\nCopyright (c) 2018-2023 J.B. Langston (MIT License)"
    "\n"
    "\nSIMH Disk Emulation (https://schorn.ch/altair.html)"
    "\nCopyright (c) 1997-2010, Charles E. Owen (MIT License)"
    "\nCopyright (c) 2002-2023, Peter Schorn (MIT License)"
    "\nCopyright (c) 2018-2023 J.B. Langston (MIT License"
    "\n"
    "\nFatFS (http://elm-chan.org/fsw/ff/)"
    "\nCopyright (C) 2022, ChaN, all right reserved (FatFS License)"
    "\n"
    "\nXmodem Protocol (https://www.menie.org/georges/embedded/#xmodem)"
    "\nCopyright 2001-2010 Georges Menie (3-clause BSD License)"
    "\nCopyright 2018-2023 J.B. Langston (3-clause BSD License)"
    "\n\n"
    "\nMIT License:"
    "\n"
    "\nPermission is hereby granted, free of charge, to any person obtaining a"
    "\ncopy of this software and associated documentation files (the \"Software\"),"
    "\nto deal in the Software without restriction, including without limitation"
    "\nthe rights to use, copy, modify, merge, publish, distribute, sublicense,"
    "\nand/or sell copies of the Software, and to permit persons to whom the"
    "\nSoftware is furnished to do so, subject to the following conditions:"
    "\n"
    "\nThe above copyright notice and this permission notice shall be included in"
    "\nall copies or substantial portions of the Software."
    "\n"
    "\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
    "\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
    "\nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
    "\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
    "\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING"
    "\nFROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER"
    "\nDEALINGS IN THE SOFTWARE."
    "\n\n"
    "\n3-clause BSD License:"
    "\n"
    "\nRedistribution and use in source and binary forms, with or without"
    "\nmodification, are permitted provided that the following conditions are met:"
    "\n"
    "\n  * Redistributions of source code must retain the above copyright"
    "\n    notice, this list of conditions and the following disclaimer."
    "\n  * Redistributions in binary form must reproduce the above copyright"
    "\n    notice, this list of conditions and the following disclaimer in the"
    "\n    documentation and/or other materials provided with the distribution."
    "\n  * Neither the name of the University of California, Berkeley nor the"
    "\n    names of its contributors may be used to endorse or promote products"
    "\n    derived from this software without specific prior written permission."
    "\n"
    "\nTHIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY"
    "\nEXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED"
    "\nWARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE"
    "\nDISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY"
    "\nDIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES"
    "\n(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;"
    "\nLOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND"
    "\nON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT"
    "\n(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS"
    "\nSOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
    "\n\n"
    "\nFatFS License:"
    "\n"
    "\nFatFs module is an open source software. Redistribution and use of FatFs in"
    "\nsource and binary forms, with or without modification, are permitted provided"
    "\nthat the following condition is met:"
    "\n"
    "\n  1. Redistributions of source code must retain the above copyright notice,"
    "\n     this condition and the following disclaimer."
    "\n"
    "\nThis software is provided by the copyright holder and contributors \"AS IS\""
    "\nand any warranties related to this software are DISCLAIMED."
    "\nThe copyright owner or contributors be NOT LIABLE for any damages caused"
    "\nby use of this software.";

/**
 * z80ctrl entry point
 */
int main(void)
{
    term_init();
    printf_P(z80ctrl_banner);
    hardware_init();
    ffw_init();
    cli_start();
}