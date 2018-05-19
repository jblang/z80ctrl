/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART declarations
 *
 * $Id: uart.h 1008 2005-12-28 21:38:59Z joerg_wunsch $
 */

/*
 * Perform UART startup initialization.
 */
void uart_init(void);

uint16_t uart_testrx (void);		/* Check number of data in UART Rx FIFO */
uint16_t uart_testtx (void);		/* Check number of data in UART Rx FIFO */
uint8_t uart_getc (void);		/* Get a byte from UART Rx FIFO */
void uart_putc (uint8_t d);		/* Put a byte into UART Tx FIFO */

/*
 * Send one character to the UART.
 */
int uart_putchar(char c, FILE * stream);

/*
 * Size of internal line buffer used by uart_getchar().
 */
#define RX_BUFSIZE 80

/*
 * Receive one character from the UART.  The actual reception is
 * line-buffered, and one character is returned from the buffer at
 * each invokation.
 */
int uart_getchar(FILE * stream);
