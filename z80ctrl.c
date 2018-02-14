#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "defines.h"
#include "uart.h"
#include "bus.h"
#include "ff.h"
#include "cli.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

FATFS fs;

int main(void)
{
    uart_init();
    stdout = stdin = &uart_str;

    puts_P(PSTR("z80ctrl 0.9 by J.B. Langston\n\n"));

    bus_init();

    cli_loop();
}