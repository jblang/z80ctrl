/* Interrupt service routine for RX complete */
#if defined(USART ## USART ## _RXC_vect)
ISR(USART ## USART ## _RXC_vect)
{
    USART_ReceiveISR(&usarts[USART]);
}
#endif

/* Interrupt service routine for Data Register Empty */
#if defined(USART ## USART ## _DRE_vect)
ISR(USART $$ USART ## _DRE_vect)
{
    USART_TransmitISR(&usarts[USART]);
}
#endif

#if defined(USART ## USART ## _TXC_vect)
ISR(USART ## USART ## _TXC_vect)
{
    usarts[USART].registers->STATUS |= USART_TXCIF_bm;
}
#define USARTS USART+1
#endif