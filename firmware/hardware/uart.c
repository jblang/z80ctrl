 /* 
 /  UART FIFO implementation is from the FatFS AVR sample code
 /  Copyright (C) 2016, ChaN, all right reserved.
 /
 / * This software is a free software and there is NO WARRANTY.
 / * No restriction on use. You can use, modify and redistribute it for
 /   any purpose as you like UNDER YOUR RESPONSIBILITY.
 / * Redistributions of source code must retain the above copyright notice.
*/

/**
 * @file uart.c Interrupt-driven buffered UART driver
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"

uint8_t watch_flag;
uint8_t watch_key;

#define	UART_BUFF  64



typedef struct {
	uint16_t	wi, ri, ct;
	uint8_t buff[UART_BUFF];
} FIFO;

static volatile FIFO TxFifo[2], RxFifo[2];

volatile uint8_t * const UCSRA[] = {&UCSR0A, &UCSR1A};
volatile uint8_t * const UCSRB[] = {&UCSR0B, &UCSR1B};
volatile uint8_t * const UBRRL[] = {&UBRR0L, &UBRR1L};
volatile uint8_t * const UBRRH[] = {&UBRR0H, &UBRR1H};
volatile uint8_t * const UDR[] = {&UDR0, &UDR1};

/* Initialize UART */

void uart_init (uint8_t uart, uint16_t ubrr)
{
    uart &= 1;

	*UCSRB[uart] = 0;

	RxFifo[uart].ct = 0; RxFifo[uart].ri = 0; RxFifo[uart].wi = 0;
	TxFifo[uart].ct = 0; TxFifo[uart].ri = 0; TxFifo[uart].wi = 0;

    *UBRRH[uart] = ubrr >> 8;
    *UBRRL[uart] = ubrr & 0xff;

	*UCSRB[uart] = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0);
}

/* Get a received character */

uint16_t uart_testrx (uint8_t uart)
{
    uart &= 1;
	return RxFifo[uart].ct;
}

uint8_t uart_getc (uint8_t uart)
{
	uint8_t d, i;
    uart &= 1;

    // Non-blocking
    if (RxFifo[uart].ct == 0)
        return 0;

	i = RxFifo[uart].ri;
	d = RxFifo[uart].buff[i];
	cli();
	RxFifo[uart].ct--;
	sei();
	RxFifo[uart].ri = (i + 1) % sizeof RxFifo[uart].buff;

	return d;
}

uint8_t uart_peek (uint8_t uart)
{
	uint8_t d, i;
    uart &= 1;

    // Non-blocking
    if (RxFifo[uart].ct == 0)
        return 0;

	i = RxFifo[uart].ri;
	d = RxFifo[uart].buff[i];

	return d;
}

/* Put a character to transmit */

uint16_t uart_testtx (uint8_t uart)
{
    uart &= 1;
	return TxFifo[uart].ct;
}

/* Flush the transmit buffers */
void uart_flush(void)
{
    while (uart_testtx(0) || uart_testtx(1))
        ;
    loop_until_bit_is_set(UCSR0A, UDRE0);
    loop_until_bit_is_set(UCSR1A, UDRE1);
 }

void uart_putc (uint8_t uart, uint8_t d)
{
	uint8_t i;
    uart &= 1;

	while (TxFifo[uart].ct >= sizeof TxFifo[uart].buff) ;

	i = TxFifo[uart].wi;
	TxFifo[uart].buff[i] = d;
	cli();
	TxFifo[uart].ct++;
	*UCSRB[uart] = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0)|_BV(UDRIE0);
	sei();
	TxFifo[uart].wi = (i + 1) % sizeof TxFifo[uart].buff;
#ifdef VDU_ANSI_EMU
    tms_putchar(d);
#endif
}

/* UART RXC interrupt */

void uart_rx_vect(uint8_t uart)
{
    uint8_t d, n, i;
    uart &= 1;

	d = *UDR[uart];
	n = RxFifo[uart].ct;
    if (watch_key && d == watch_key)
        watch_flag = 1;
	if (n < sizeof RxFifo[uart].buff) {
		RxFifo[uart].ct = ++n;
		i = RxFifo[uart].wi;
		RxFifo[uart].buff[i] = d;
		RxFifo[uart].wi = (i + 1) % sizeof RxFifo[uart].buff;
	}
}

ISR(USART0_RX_vect)
{
    uart_rx_vect(0);
}

ISR(USART1_RX_vect)
{
    uart_rx_vect(1);
}


/* UART UDRE interrupt */

void uart_udre_vect(uint8_t uart)
{
	uint8_t n, i;
    uart &= 1;

	n = TxFifo[uart].ct;
	if (n) {
		TxFifo[uart].ct = --n;
		i = TxFifo[uart].ri;
		*UDR[uart] = TxFifo[uart].buff[i];
		TxFifo[uart].ri = (i + 1) % sizeof TxFifo[uart].buff;
	}
	if (n == 0) *UCSRB[uart] = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0);
}

ISR(USART0_UDRE_vect)
{
    uart_udre_vect(0);
}

ISR(USART1_UDRE_vect)
{
    uart_udre_vect(1);
}