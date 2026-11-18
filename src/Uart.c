#include "../include/Uart.h"
#include <ctype.h>

static FILE uart_output = FDEV_SETUP_STREAM(uart_putch, NULL, _FDEV_SETUP_WRITE);
static FILE uart1_output = FDEV_SETUP_STREAM(uart1_putch, NULL, _FDEV_SETUP_WRITE);

void RS232Init(void)
{
    UBRR0L = (unsigned char)(MyUBRR & 0xff);
    UBRR0H = (unsigned char)(MyUBRR >> 8);
#ifdef UART_NORMAL_MODE
    UCSR0A = 0x00;
#else
    UCSR0A = (1 << U2X0);
#endif
    /* RX + TX + RX-interrupt (USART0) */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    stdout = &uart_output;
    Uart1Init();
}

void Uart1Init(void)
{
    UBRR1L = (unsigned char)(MyUBRR & 0xff);
    UBRR1H = (unsigned char)(MyUBRR >> 8);
#ifdef UART_NORMAL_MODE
    UCSR1A = 0x00;
#else
    UCSR1A = (1 << U2X1);
#endif
    /* RX + TX + RX-interrupt (USART1) */
    UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

int uart_putch(char ch, FILE *stream)
{
    (void)stream;
    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = ch;
    return 0;
}

int uart1_putch(char ch, FILE *stream)
{
    (void)stream;
    while (!(UCSR1A & (1 << UDRE1))) {
    }
    UDR1 = ch;
    return 0;
}

FILE *uart1_stream(void)
{
    return &uart1_output;
}

void uart0_print(const char *text)
{
    while (text != NULL && *text != '\0') {
        uart_putch(*text++, NULL);
    }
}

void uart1_print(const char *text)
{
    while (text != NULL && *text != '\0') {
        uart1_putch(*text++, NULL);
    }
}

void uart_print_both(const char *text)
{
    uart0_print(text);
    uart1_print(text);
}

/*
 * Opgavekrav: UART0 RX laver bogstaver om til STORE bogstaver
 * og sender tegnet tilbage på UART0.
 */
ISR(USART0_RX_vect)
{
    char received = (char)UDR0;

    if (islower((unsigned char)received)) {
        received = (char)toupper((unsigned char)received);
    }

    while (!(UCSR0A & (1 << UDRE0))) {
    }
    UDR0 = received;
}

/*
 * Opgavekrav: UART1 RX laver bogstaver om til små bogstaver
 * og sender tegnet tilbage på UART1.
 */
ISR(USART1_RX_vect)
{
    char received = (char)UDR1;

    if (isupper((unsigned char)received)) {
        received = (char)tolower((unsigned char)received);
    }

    while (!(UCSR1A & (1 << UDRE1))) {
    }
    UDR1 = received;
}
