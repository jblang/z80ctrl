#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include <util/delay.h>

#include "uart.h"
#include "bus.h"

#include "hello.h"
#include "turnmon.h"
//#include "basic4k40.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void)
{
    char buf[80];
    char *sep = " \t";
    char *tok;
    int i = 0;

    uart_init();
    stdout = stdin = &uart_str;
    _delay_ms(500);

    bus_init();
    printf("bus initialized: ");
    bus_status();

    printf("copying program...\n");
    write_mem(0x0100, hello_bin, hello_bin_len);
    write_mem(0xfd00, turnmon_bin, turnmon_bin_len);

    printf("running program...\n");
    z80_run(0xfd00);

    printf("halted.\n");

    for (;;) {
    }
}