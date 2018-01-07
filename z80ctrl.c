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
#include <avr/interrupt.h>

#include <util/delay.h>

#include "uart.h"
#include "bus.h"


FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

unsigned char hello_bin[] = {
  0x21, 0x10, 0x00, 0xcd, 0x08, 0x00, 0x18, 0xfe, 0x7e, 0xa7, 0xc8, 0xd3,
  0x00, 0x23, 0x18, 0xf8, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
  0x6f, 0x72, 0x6c, 0x64, 0x00
};
unsigned int hello_bin_len = 29;

int main(void)
{
    uart_init();
    stdout = stdin = &uart_str;

    _delay_ms(5000);
    printf("starting...\n");

    bus_init();
    clk_run();
    _delay_ms(1);
    reset_hi();

    printf("copy program...\n");
    bus_master();
    write_mem(0x0000, hello_bin, hello_bin_len);
    bus_slave();

    printf("reset...\n");
    reset_lo();
    _delay_ms(1);
    clk_stop();

    printf("debugging...\n");
    reset_hi();
    bus_trace(1500);
    return 0;
}
