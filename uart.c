/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file uart.c Interrupt-driven buffered UART driver with cooked mode terminal
 */

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#ifdef TMS_BASE
#include "tms.h"
#endif

#define	UART_BUFF  64
#define LINE_BUFF  80

 /* 
 /  UART FIFO implementation is from the FatFS AVR sample code
 /  Copyright (C) 2016, ChaN, all right reserved.
 /
 / * This software is a free software and there is NO WARRANTY.
 / * No restriction on use. You can use, modify and redistribute it for
 /   any purpose as you like UNDER YOUR RESPONSIBILITY.
 / * Redistributions of source code must retain the above copyright notice.
*/

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
#ifdef TMS_BASE
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

/*
 * "Cooked" terminal mode functions are from libc-avr stdio demo
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int uart_putchar(char c, FILE * stream)
{

    if (c == '\n')
        uart_putchar('\r', stream);
    uart_putc(0, c);

    return 0;
}

/*
 * Receive a character from the UART Rx.
 *
 * This features a simple line-editor that allows to delete and
 * re-edit the characters entered, until either CR or NL is entered.
 * Printable characters entered will be echoed using uart_putchar().
 *
 * Editing characters:
 *
 * . \b (BS) or \177 (DEL) delete the previous character
 * . ^u kills the entire input buffer
 * . ^w deletes the previous word
 * . ^r sends a CR, and then reprints the buffer
 * . \t will be replaced by a single space
 *
 * All other control characters will be ignored.
 *
 * The internal line buffer is RX_BUFSIZE (80) characters long, which
 * includes the terminating \n (but no terminating \0).  If the buffer
 * is full (i. e., at RX_BUFSIZE-1 characters in order to keep space for
 * the trailing \n), any further input attempts will send a \a to
 * uart_putchar() (BEL character), although line editing is still
 * allowed.
 *
 * Input errors while talking to the UART will cause an immediate
 * return of -1 (error indication).  Notably, this will be caused by a
 * framing error (e. g. serial line "break" condition), by an input
 * overrun, and by a parity error (if parity was enabled and automatic
 * parity recognition is supported by hardware).
 *
 * Successive calls to uart_getchar() will be satisfied from the
 * internal buffer until that buffer is emptied again.
 */
int uart_getchar(FILE * stream)
{
    uint8_t c;
    char *cp, *cp2;
    static char b[LINE_BUFF];
    static char *rxp;

    if (rxp == 0)
        for (cp = b;;) {
            // block for character
            while (uart_testrx(0) == 0)
                ;
            c = uart_getc(0);
            /* behaviour similar to Unix stty ICRNL */
            if (c == '\r')
                c = '\n';
            if (c == '\n') {
                *cp = c;
                uart_putchar(c, stream);
                rxp = b;
                break;
            } else if (c == '\t')
                c = ' ';

            if ((c >= (uint8_t) ' ' && c <= (uint8_t) '\x7e') ||
                c >= (uint8_t) '\xa0') {
                if (cp == b + LINE_BUFF - 1)
                    uart_putchar('\a', stream);
                else {
                    *cp++ = c;
                    uart_putchar(c, stream);
                }
                continue;
            }

            switch (c) {
            case 'c' & 0x1f:
                return -1;

            case '\b':
            case '\x7f':
                if (cp > b) {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'r' & 0x1f:
                uart_putchar('\r', stream);
                for (cp2 = b; cp2 < cp; cp2++)
                    uart_putchar(*cp2, stream);
                break;

            case 'u' & 0x1f:
                while (cp > b) {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'w' & 0x1f:
                while (cp > b && cp[-1] != ' ') {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;
            }
        }

    c = *rxp++;
    if (c == '\n')
        rxp = 0;

    return c;
}