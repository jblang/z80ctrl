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
  0x21, 0x13, 0x00, 0xcd, 0x0b, 0x00, 0xdb, 0x00, 0xd3, 0x00, 0x76, 0x7e,
  0xa7, 0xc8, 0xd3, 0x00, 0x23, 0x18, 0xf8, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
  0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x0a, 0x00
};
unsigned int hello_bin_len = 33;

int main(void)
{
    unsigned char hello_read[hello_bin_len];
    uint8_t i;
    
    uart_init();
    stdout = stdin = &uart_str;
    _delay_ms(500);

    bus_init();
    printf("bus initialized: ");
    bus_status();

    printf("copying program...\n");
    bus_master();
    write_mem(0x0000, hello_bin, hello_bin_len);

    printf("verifying program...\n");
    read_mem(0x0000, hello_read, hello_bin_len);
    uint8_t error = 0;
    for (i = 0; i < hello_bin_len; i++) {
      if (hello_bin[i] != hello_read[i]) {
        printf("error at %x: %x (read) != %x (orig)\n", i, hello_read[i], hello_bin[i]);
        error = 1;
      }
    }
    if (error)
      printf("verification failed!\n");
    else
      printf("program verified.\n");
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
          case 0x00:
            if (!GET_WR) {
                putchar(GET_DATA);
            } else if (!GET_RD) {
                SET_DATA(getchar());
                DATA_OUTPUT;
            }
            break;
      }
      IOACK_LO;
      while(!GET_IORQ)
        CLK_TOGGLE;
      DATA_INPUT;
      IOACK_HI;
    }

}