#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "uart.h"
#include "bus.h"
#include "ff.h"

#include "hello.h"
#include "altmon.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

#define WHITESPACE " \t\r\n"
#define MAXBUF 80
#define MAXARGS 3

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

void command_line(void) {
    char buf[MAXBUF];
    char *cmd;
    char *args[MAXARGS];
    int numargs;
    uint16_t addr;
    uint16_t length;

    for (;;) {
        printf("> ");
        if (fgets(buf, sizeof buf - 1, stdin) != NULL) {
            if ((cmd = strtok(buf, WHITESPACE)) == NULL)
                continue;            
            for (numargs = 0; numargs < MAXARGS; numargs++) {
                if ((args[numargs] = strtok(NULL, WHITESPACE)) == NULL)
                    break;
            }
            if (strcmp(cmd, "dump") == 0) {
                if (numargs != 2) {
                    printf("usage: dump <start> <length>\n");
                    continue;
                }
                sscanf(args[0], "%x", &addr);
                sscanf(args[1], "%x", &length);
                dump_mem(addr, length);
            } else if (strcmp(cmd, "run") == 0) {
                if (numargs != 1) {
                    printf("usage: run <address>\n");
                }
                sscanf(args[0], "%x", &addr);
                z80_run(addr);
            } else if (strcmp(cmd, "bus") == 0) {
                bus_status();
            } else {
                printf("unknown command: %s\n", cmd);
            }
        }
    }
}

int main(void)
{

    uart_init();
    stdout = stdin = &uart_str;

    printf("Z80Ctrl 0.9 by J.B. Langston\n\n");

    printf("initializing sd card...\n");
    sd_init();
    
    printf("initializing bus...\n");
    bus_init();

    printf("copying monitor...\n");
    write_mem(0xf800, altmon_bin, altmon_bin_len);

    printf("starting monitor...\n");
    z80_run(0xf800);

    printf("halted.\n");

    command_line();
}