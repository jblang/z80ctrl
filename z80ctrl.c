#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "defines.h"
#include "uart.h"
#include "bus.h"
#include "ff.h"
#include "cli.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

volatile UINT Timer;    /* Performance timer (100Hz increment) */
FATFS fs;

static
void sd_init (void)
{
    FRESULT rc;

    /* Start 100Hz system timer with TC0 */
    OCR0A = F_CPU / 1024 / 100 - 1;
    TCCR0A = _BV(WGM01);
    TCCR0B = 0b101;
    TIMSK0 = _BV(OCIE0A);

    sei();
    disk_initialize(0);
    if ((rc = f_mount(&fs, "", 1)) != FR_OK) {
        printf("error %d mounting fs.", rc);
    }
}

ISR(TIMER0_COMPA_vect)
{
    Timer++;                        /* Performance counter for this module */
    disk_timerproc();       /* Drive timer procedure of low level disk I/O module */
}

DWORD get_fattime (void) {
    return 0;
}

int main(void)
{

    uart_init();
    stdout = stdin = &uart_str;

    printf("z80ctrl 0.9 by J.B. Langston\n\n");

    sd_init();
    bus_init();

    cli_loop();
}