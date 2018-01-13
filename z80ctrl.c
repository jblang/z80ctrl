#include <avr/io.h>
#include <avr/interrupt.h>
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
    FRESULT rc;

    uart_init();
    stdout = stdin = &uart_str;

    printf("z80ctrl 0.9 by J.B. Langston\n\n");

    disk_initialize(0);
    if ((rc = f_mount(&fs, "", 1)) != FR_OK) {
        printf("error %d mounting fs.", rc);
    }
    bus_init();

    cli_loop();
}