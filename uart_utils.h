#ifndef UART_UTILS_H_
#define UART_UTILS_H_

#include <stm32f4xx.h>
#include <stdarg.h>
#include <stdio.h>

extern uint32_t SystemCoreClock; // system clock frequency

// convert baud into BRR value
#define baud(bps) \
	(((SystemCoreClock/((bps)*16)) << 4) | ((SystemCoreClock/(bps)) % 16))

// Funktioprototyypit
void uart_init(void);
void uart_send(char c);
void uart_send_str(char *s);
char uart_recv(void);
void print(const char *format, ...);
void println(const char *format, ...);
float read_float_from_uart(float min, float max);

#endif /* UART_UTILS_H_ */
