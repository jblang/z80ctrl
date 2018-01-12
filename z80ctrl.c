#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>

#include <util/delay.h>

#include "uart.h"
#include "bus.h"

#include "hello.h"
//#include "turnmon.h"
//#include "basic4k40.h"
#include "altmon.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

#define WHITESPACE " \t\r\n"
#define MAXBUF 80
#define MAXARGS 3

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
    _delay_ms(500);

    bus_init();
    printf("bus initialized: ");
    bus_status();

    printf("copying program...\n");
    write_mem(0x0100, hello_bin, hello_bin_len);
    write_mem(0xf800, altmon_bin, altmon_bin_len);

    printf("running program...\n");
    z80_run(0xf800);

    printf("halted.\n");

    command_line();
}