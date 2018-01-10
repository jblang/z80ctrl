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

#include "hello.h"
#include "turnmon.h"
//#include "basic4k40.h"
#define PROG_BIN turnmon_bin
#define PROG_ORG 0xFD00
#define PROG_LEN turnmon_bin_len

uint8_t reset_vect[] = {0xc3, (PROG_ORG & 0xFF), (PROG_ORG >> 8)};


FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void)
{
    unsigned char verify_buf[PROG_LEN];
    int i;
    
    uart_init();
    stdout = stdin = &uart_str;
    _delay_ms(500);

    bus_init();
    printf("bus initialized: ");
    bus_status();

    printf("copying program...\n");
    bus_master();
#if (PROG_ORG != 0x0000)
    write_mem(0x0000, reset_vect, 3);
#endif
    write_mem(PROG_ORG, PROG_BIN, PROG_LEN);
    write_mem(0x100, hello_bin, hello_bin_len);

    bus_slave();

    printf("resetting processor...\n");
    z80_reset();

    //bus_trace(1000);
    for (;;) {
      clk_run();
      while(GET_IORQ)
        ;
      clk_stop();
      switch (GET_ADDRLO) {
          case 0x10:    // sio0 status
            if (!GET_RD) {
              SET_DATA(((UCSR0A >> (UDRE0-1)) & 0x2) | ((UCSR0A >> RXC0) & 0x1));
              DATA_OUTPUT;
            }
            break;
          case 0x11:    // sio0 data
            if (!GET_RD) {
              SET_DATA(UDR0);
              DATA_OUTPUT;
            } else if (!GET_WR) {
              UDR0 = GET_DATA;
            }
            break;
          default:
            if (!GET_RD) {
              SET_DATA(0377);
              DATA_OUTPUT;
            }
      }
      IOACK_LO;
      while(!GET_IORQ)
        CLK_TOGGLE;
      DATA_INPUT;
      IOACK_HI;
    }
}