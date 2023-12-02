/*
 * "Cooked" terminal mode functions are from libc-avr stdio demo
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 */

/**
 * @file term.c Cooked terminal mode functions
 */

#include <stdio.h>

#include "uart.h"

#define LINE_BUFF  80

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int term_putchar(char c, FILE * stream)
{

    if (c == '\n')
        term_putchar('\r', stream);
    uart_putc(0, c);

    return 0;
}

/*
 * Receive a character from the UART Rx.
 *
 * This features a simple line-editor that allows to delete and
 * re-edit the characters entered, until either CR or NL is entered.
 * Printable characters entered will be echoed using term_putchar().
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
 * term_putchar() (BEL character), although line editing is still
 * allowed.
 *
 * Input errors while talking to the UART will cause an immediate
 * return of -1 (error indication).  Notably, this will be caused by a
 * framing error (e. g. serial line "break" condition), by an input
 * overrun, and by a parity error (if parity was enabled and automatic
 * parity recognition is supported by hardware).
 *
 * Successive calls to term_getchar() will be satisfied from the
 * internal buffer until that buffer is emptied again.
 */
int term_getchar(FILE * stream)
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
                term_putchar(c, stream);
                rxp = b;
                break;
            } else if (c == '\t')
                c = ' ';

            if ((c >= (uint8_t) ' ' && c <= (uint8_t) '\x7e') ||
                c >= (uint8_t) '\xa0') {
                if (cp == b + LINE_BUFF - 1)
                    term_putchar('\a', stream);
                else {
                    *cp++ = c;
                    term_putchar(c, stream);
                }
                continue;
            }

            switch (c) {
            case 'c' & 0x1f:
                return -1;

            case '\b':
            case '\x7f':
                if (cp > b) {
                    term_putchar('\b', stream);
                    term_putchar(' ', stream);
                    term_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'r' & 0x1f:
                term_putchar('\r', stream);
                for (cp2 = b; cp2 < cp; cp2++)
                    term_putchar(*cp2, stream);
                break;

            case 'u' & 0x1f:
                while (cp > b) {
                    term_putchar('\b', stream);
                    term_putchar(' ', stream);
                    term_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'w' & 0x1f:
                while (cp > b && cp[-1] != ' ') {
                    term_putchar('\b', stream);
                    term_putchar(' ', stream);
                    term_putchar('\b', stream);
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

FILE term_stream = FDEV_SETUP_STREAM(term_putchar, term_getchar, _FDEV_SETUP_RW);

void term_init()
{
    uart_init(0, UBRR115200);
    uart_init(1, UBRR115200);
    stderr = stdout = stdin = &term_stream;
}