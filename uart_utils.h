#ifndef UART_UTILS_H_
#define UART_UTILS_H_

#include <stm32f4xx.h>

extern uint32_t SystemCoreClock; // system clock frequency

// convert baud into BRR value
#define baud(bps) \
	(((SystemCoreClock/((bps)*16)) << 4) | ((SystemCoreClock/(bps)) % 16))

// Funktioprototyypit
void uart_init(void);
void uart_send(char c);
void uart_send_str(char *s);
char uart_recv(void);

uint32_t uart_recv_int_until_d(void);
float uart_recv_float_until_d(void);

#endif /* UART_UTILS_H_ */
