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

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

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
static
volatile FIFO TxFifo, RxFifo;

/* Initialize UART */

void uart_init (uint32_t bps)
{
	uint16_t n;


	UCSR0B = 0;

	RxFifo.ct = 0; RxFifo.ri = 0; RxFifo.wi = 0;
	TxFifo.ct = 0; TxFifo.ri = 0; TxFifo.wi = 0;

	n = F_CPU / bps / 8;
	UBRR0L = (n >> 1) + (n & 1) - 1;
	UCSR0B = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0);
}

/* Get a received character */

uint16_t uart_testrx (void)
{
	return RxFifo.ct;
}

uint8_t uart_getc (void)
{
	uint8_t d, i;

    // Non-blocking
    if (RxFifo.ct == 0)
        return 0;

	i = RxFifo.ri;
	d = RxFifo.buff[i];
	cli();
	RxFifo.ct--;
	sei();
	RxFifo.ri = (i + 1) % sizeof RxFifo.buff;

	return d;
}

/* Put a character to transmit */

uint16_t uart_testtx (void)
{
	return TxFifo.ct;
}

void uart_putc (uint8_t d)
{
	uint8_t i;


	while (TxFifo.ct >= sizeof TxFifo.buff) ;

	i = TxFifo.wi;
	TxFifo.buff[i] = d;
	cli();
	TxFifo.ct++;
	UCSR0B = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0)|_BV(UDRIE0);
	sei();
	TxFifo.wi = (i + 1) % sizeof TxFifo.buff;
}


/* UART RXC interrupt */

ISR(USART0_RX_vect)
{
	uint8_t d, n, i;


	d = UDR0;
	n = RxFifo.ct;
	if (n < sizeof RxFifo.buff) {
		RxFifo.ct = ++n;
		i = RxFifo.wi;
		RxFifo.buff[i] = d;
		RxFifo.wi = (i + 1) % sizeof RxFifo.buff;
	}
}


/* UART UDRE interrupt */

ISR(USART0_UDRE_vect)
{
	uint8_t n, i;


	n = TxFifo.ct;
	if (n) {
		TxFifo.ct = --n;
		i = TxFifo.ri;
		UDR0 = TxFifo.buff[i];
		TxFifo.ri = (i + 1) % sizeof TxFifo.buff;
	}
	if (n == 0) UCSR0B = _BV(RXEN0)|_BV(RXCIE0)|_BV(TXEN0);
}

/*
 * uart_getchar and uart_putchar functions are from libc-avr stdio demo
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
    uart_putc(c);

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
            while (uart_testrx() == 0)
                ;
            c = uart_getc();
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