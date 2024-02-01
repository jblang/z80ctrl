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
 * @file sn76489.h SN76489 sound chip functions
 */

#include <util/delay.h>

#include "bus.h"

/**
 * Mute the SN76489
 */
void sn76489_mute()
{
#ifdef SN76489_PORT
    if (GET_BUSACK)
        return;
    uint8_t oldclkdiv = get_clkdiv();
    set_clkdiv(4);
    clk_run();
    DATA_OUTPUT;
    SET_ADDRLO(SN76489_PORT);
    IORQ_LO;
    SET_DATA(0x9F);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xBF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xDF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    SET_DATA(0xFF);
    WR_LO;
    _delay_us(10);
    WR_HI;
    IORQ_HI;
    clk_stop();
    set_clkdiv(oldclkdiv);
#endif
}