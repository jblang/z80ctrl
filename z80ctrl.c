/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo
 *
 * $Id: stdiodemo.c 1008 2005-12-28 21:38:59Z joerg_wunsch $
 */

#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "uart.h"
#include "spi.h"
#include "iox.h"
#include "bus.h"

FILE uart_str =
FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

uint8_t prog[] = {
    0x3a, 0x0d, 0x00, 0x2a, 0x0e, 0x00, 0x86, 0x32, 0x0f, 0x00, 0xc3, 0x0a, 0x00, 0x0a, 0x05, 0x00
};

int main(void)
{
    char buf[80];
    _delay_ms(10000);
    uart_init();
    stdout = stdin = &uart_str;

    spi_init((1 << SD_CS) | (1 << IOX_CS));
    bus_init();
    clk_run();
    reset_hi();
    bus_master();
    write_mem(0x0000, prog, sizeof prog);
    reset_lo();
    bus_slave();
    clk_stop();
    reset_hi();
    uint8_t i;
    for (i = 0;; i++) {
        clk_tick();
    }
    return 0;
}
