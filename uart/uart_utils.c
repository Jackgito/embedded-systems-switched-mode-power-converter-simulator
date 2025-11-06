/**
 ******************************************************************************
 * @file    uart_utils.c
 * @brief   UART (USART2) utility functions for STM32 + printf() redirection
 *
 * @details
 *  - Initializes USART2 (PA2 = TX, PA3 = RX)
 *  - Provides simple send/receive functions
 *  - Redirects standard output (printf) via _write()
 ******************************************************************************
 */

#include "uart_utils.h"
#include "bitwise.h"
#include <stm32f4xx.h>
#include <stdio.h>

extern uint32_t SystemCoreClock; // System clock frequency

// Convert baud rate into BRR value for USART
#define baud(bps) \
	(((SystemCoreClock/((bps)*16)) << 4) | ((SystemCoreClock/(bps)) % 16))

/**
 * @brief Initialize USART2 for 115200 8N1
 */
void uart_init(void)
{
	// Enable GPIOA and USART2 clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	// Configure PA2 (TX) and PA3 (RX) as AF7 (USART2)
	bits_val(GPIOA->AFR[0], 4, 2, 7); // PA2 -> USART2_TX
	bits_val(GPIOA->AFR[0], 4, 3, 7); // PA3 -> USART2_RX
	bits_val(GPIOA->MODER , 2, 2, 2); // PA2 -> Alternate Function
	bits_val(GPIOA->MODER , 2, 3, 2); // PA3 -> Alternate Function

	// Configure USART2: 115200 baud, 8 data bits, 1 stop bit, no parity
	USART2->BRR  = baud(115200);
	USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/**
 * @brief Send single character via UART (blocking)
 */
void uart_send(char c)
{
	while (!(USART2->SR & USART_SR_TXE)); // Wait until TX buffer empty
	USART2->DR = c;
}

/**
 * @brief Send string via UART (null-terminated)
 */
void uart_send_str(const char *s)
{
	while (*s)
		uart_send(*s++);
}

/**
 * @brief Receive single character via UART (blocking)
 */
char uart_recv(void)
{
	while (!(USART2->SR & USART_SR_RXNE)); // Wait for received data
	return (char)USART2->DR;
}

/**
 * @brief  Retargets the C library printf() to USART2
 * @note   Called automatically when printf() is used.
 */
int _write(int file, char *buffer, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (buffer[i] == '\n')
			uart_send('\r'); // Add carriage return for terminal compatibility

		uart_send(buffer[i]);
	}
	return size;
}
