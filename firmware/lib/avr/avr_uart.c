/*
/  UART FIFO implementation is from the FatFS AVR sample code
/  Copyright (C) 2016, ChaN, all right reserved.
/  Copyright (C) 2018-2024 J.B. Langston
/
/  Modified by J.B. Langston to support multiple UARTs and modern AVRs.
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

#include "avr_uart.h"

uint8_t watch_flag;
uint8_t watch_key;

#define UART_BUFF 64

typedef struct
{
    uint16_t wi, ri, ct;
    uint8_t buff[UART_BUFF];
} FIFO;

static volatile FIFO TxFifo[2], RxFifo[2];

#if __AVR_ARCH__ < 100 // Classic AVRs

volatile uint8_t *const CTRLA[] = {&UCSR0A, &UCSR1A};
volatile uint8_t *const CTRLB[] = {&UCSR0B, &UCSR1B};
volatile uint8_t *const CTRLC[] = {&UCSR0C, &UCSR1C};
volatile uint8_t *const BAUDL[] = {&UBRR0L, &UBRR1L};
volatile uint8_t *const BAUDH[] = {&UBRR0H, &UBRR1H};
volatile uint8_t *const DATA[] = {&UDR0, &UDR1};

#define UART_ENABLE_RXTX(n) *CTRLB[n] |= ((1 << RXEN0) | (1 << TXEN0))
#define UART_DISABLE_RXTX(n) *CTRLB[n] &= ~((1 << RXEN0) | (1 << TXEN0))
#define UART_ENABLE_RXINT(n) *CTRLB[n] |= (1 << RXCIE0)
#define UART_DISABLE_RXINT(n) *CTRLB[n] &= ~(1 << RXCIE0)
#define UART_ENABLE_TXINT(n) *CTRLB[n] |= (1 << UDRIE0)
#define UART_DISABLE_TXINT(n) *CTRLB[n] &= ~(1 << UDRIE0)
#define UART_WAIT(n) loop_until_bit_is_set(*CTRLA[n], UDRE0);
#define UART_TX(n, d) *DATA[n] = d
#define UART_RX(n) *DATA[n]

#define BAUD_TOL 2
void uart_set_baud(uint8_t n, uint32_t baud)
{
    uint16_t ubrr = ((F_CPU + 8UL * baud) / (16UL * baud) - 1UL);
    uint32_t tol = ((16 * (ubrr + 1)) * (100 * baud + baud * (BAUD_TOL)));
    if (100 * F_CPU > tol || 100 * F_CPU < tol) {
        *CTRLA[n] |= (1 << U2X0);
        ubrr = ((F_CPU + 4UL * baud) / (8UL * baud) - 1UL);
    } else {
        *CTRLA[n] &= ~(1 << U2X0);
    }
    *BAUDL[n] = ubrr;
    *BAUDH[n] = ubrr >> 8;
}

#else // Modern AVRs (xmega register architecture)

volatile USART_t *const USART[] = {&USART2, &USART1};
#define UART_ENABLE_RXTX(n) USART[n]->CTRLB |= USART_RXEN_bm | USART_TXEN_bm
#define UART_DISABLE_RXTX(n) USART[n]->CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm)
#define UART_ENABLE_RXINT(n) USART[n]->CTRLA |= USART_RXCIE_bm
#define UART_DISABLE_RXINT(n) USART[n]->CTRLA &= ~USART_RXCIE_bm
#define UART_ENABLE_TXINT(n) USART[n]->CTRLA |= USART_DREIE_bm
#define UART_DISABLE_TXINT(n) USART[n]->CTRLA &= ~USART_DREIE_bm
#define UART_WAIT(n) loop_until_bit_is_set(USART[n]->STATUS, USART_DREIF_bp);
#define UART_TX(n, d) USART[n]->TXDATAL = d
#define UART_RX(n) USART[n]->RXDATAL

void uart_set_baud(uint8_t n, uint32_t baud)
{
    USART[n]->BAUD = ((uint16_t)((float)(64 * F_CPU / (16 * (float)(baud))) + 0.5));
}

#endif

/* Initialize UART */

void uart_init(uint8_t uart, uint32_t baud)
{
    uart &= 1;

    UART_DISABLE_RXTX(uart);

    RxFifo[uart].ct = 0;
    RxFifo[uart].ri = 0;
    RxFifo[uart].wi = 0;
    TxFifo[uart].ct = 0;
    TxFifo[uart].ri = 0;
    TxFifo[uart].wi = 0;

    uart_set_baud(uart, baud);
    UART_ENABLE_RXINT(uart);
    UART_ENABLE_RXTX(uart);
}

/* Get a received character */

uint16_t uart_testrx(uint8_t uart)
{
    uart &= 1;
    return RxFifo[uart].ct;
}

uint8_t uart_getc(uint8_t uart)
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

uint8_t uart_peek(uint8_t uart)
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

uint16_t uart_testtx(uint8_t uart)
{
    uart &= 1;
    return TxFifo[uart].ct;
}

/* Flush the transmit buffers */
void uart_flush(void)
{
    while (uart_testtx(0) || uart_testtx(1))
        ;
    UART_WAIT(0);
    UART_WAIT(1);
}

void uart_putc(uint8_t uart, uint8_t d)
{
    uint8_t i;
    uart &= 1;

    while (TxFifo[uart].ct >= sizeof TxFifo[uart].buff)
        ;

    i = TxFifo[uart].wi;
    TxFifo[uart].buff[i] = d;
    cli();
    TxFifo[uart].ct++;
    UART_ENABLE_TXINT(uart);
    sei();
    TxFifo[uart].wi = (i + 1) % sizeof TxFifo[uart].buff;
}

/* UART RXC interrupt */

void uart_rx_next(uint8_t uart)
{
    uint8_t d, n, i;
    uart &= 1;

    d = UART_RX(uart);
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
    uart_rx_next(0);
}

ISR(USART1_RX_vect)
{
    uart_rx_next(1);
}

/* UART UDRE interrupt */

void uart_tx_next(uint8_t uart)
{
    uint8_t n, i;
    uart &= 1;

    n = TxFifo[uart].ct;
    if (n) {
        TxFifo[uart].ct = --n;
        i = TxFifo[uart].ri;
        UART_TX(uart, TxFifo[uart].buff[i]);
        TxFifo[uart].ri = (i + 1) % sizeof TxFifo[uart].buff;
    }
    if (n == 0)
        UART_DISABLE_TXINT(n);
}

ISR(USART0_UDRE_vect)
{
    uart_tx_next(0);
}

ISR(USART1_UDRE_vect)
{
    uart_tx_next(1);
}