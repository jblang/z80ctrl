/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART implementation
 *
 * $Id: uart.c 1008 2005-12-28 21:38:59Z joerg_wunsch $
 * 
 * uart_getc and uart_putc functions from FatFs AVR sample by ChaN
 * (C)ChaN, 2017
 * 
 */

#include "defines.h"

#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include "uart.h"

#define	UART_BUFF		64


typedef struct {
	uint16_t	wi, ri, ct;
	uint8_t buff[UART_BUFF];
} FIFO;
static
volatile FIFO TxFifo, RxFifo;

/*
 * Initialize the UART to 9600 Bd, tx/rx, 8N1.
 */
void uart_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

/* Get a received character */

uint16_t uart_testrx (void)
{
	return RxFifo.ct;
}

uint16_t uart_testtx (void)
{
	return TxFifo.ct;
}

uint8_t uart_getc (void)
{
	uint8_t d, i;


	while (RxFifo.ct == 0) ;

	i = RxFifo.ri;
	d = RxFifo.buff[i];
	cli();
	RxFifo.ct--;
	sei();
	RxFifo.ri = (i + 1) % sizeof RxFifo.buff;

	return d;
}


/* Put a character to transmit */

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
    static char b[UART_BUFF];
    static char *rxp;

    if (rxp == 0)
        for (cp = b;;) {
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
                if (cp == b + UART_BUFF - 1)
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

