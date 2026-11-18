#ifndef UART_H_
#define UART_H_

#include "ProjectDefines.h"
#include <stdio.h>

#define BAUD 9600UL

#define UART_NORMAL_MODE
#ifdef UART_NORMAL_MODE
#define MyUBRR (unsigned int)(F_CPU / (16UL * BAUD) - 1UL)
#else
#define MyUBRR (unsigned int)(F_CPU / (8UL * BAUD) - 1UL)
#endif

void RS232Init(void);
void Uart1Init(void);

int uart_putch(char ch, FILE *stream);
int uart1_putch(char ch, FILE *stream);
FILE *uart1_stream(void);

void uart0_print(const char *text);
void uart1_print(const char *text);
void uart_print_both(const char *text);

#endif
