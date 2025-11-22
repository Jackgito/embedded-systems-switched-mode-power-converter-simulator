#include "uart_utils.h"
#include "bitwise.h" // Oletetaan, että bitwise.h on projektin polussa

// Alustaa USART2:n
void uart_init(void)
{
	// set PA2 & PA3 alternate functions to AF7 (USART2 RX/TX)
	bits_val(GPIOA->AFR[0], 4, 2, 7); // PA2 -> USART2_TX
	bits_val(GPIOA->AFR[0], 4, 3, 7); // PA3 -> USART2_RX
	bits_val(GPIOA->MODER , 2, 2, 2); // PA2 -> alternate function mode
	bits_val(GPIOA->MODER , 2, 3, 2); // PA3 -> alternate function mode

	// configure UART as 8N1 at 115200bps
	USART2->BRR  = baud(115200);
	USART2->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
}

// send single character to UART
void uart_send(char c)
{
	// wait for the transmit buffer to become empty
	while (!(USART2->SR & USART_SR_TXE))
		;

	// begin transmission of character
	USART2->DR = c;
}

// send string to UART
void uart_send_str(char *s)
{
	char c;

	// transmit whole string
	while ((c = *s++) != '\0')
		uart_send(c);
}

// receive single character from UART (blocking)
char uart_recv()
{
	// wait for a single byte to be received
	while (!(USART2->SR & USART_SR_RXNE))
		;

	// return received byte
	return USART2->DR;
}
