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


// Lukee kokonaisluvun (esim. "123d")
uint32_t uart_recv_int_until_d(void)
{
    uint32_t value = 0;
    char c;
    while (1)
    {
        c = uart_recv();
        if (c >= '0' && c <= '9') {
            uart_send(c); // Ilmoittaa annetun numeron terminaalista
            value = value * 10 + (c - '0');
        }
        else if (c == 'd') {
            uart_send_str("\r\n");
            break;
        }
    }
    return value;
}

// Lukee desimaaliluvun (esim. "0.05d" tai "1.2d")
float uart_recv_float_until_d(void)
{
    float result = 0.0f;
    float fraction_div = 1.0f; // Jos käyttäjä haluaa desimaaliluvun, niin tällä voidaan jakaa annettu arvo desimaaleiksi.
    int is_fraction = 0; // Käytetään jakamisprosessin aloitukseen
    char c;

    while (1)
    {
        c = uart_recv();
        if (c >= '0' && c <= '9')
        {
            uart_send(c); // Ilmoittaa annetun arvon terminaalissa
            if (is_fraction==0) {
                result = result * 10.0f + (c - '0');
            } else {
                fraction_div *= 10.0f;
                result += (float)(c - '0') / fraction_div;
            }
        }
        else if (c == '.' && is_fraction==0)
        {
            uart_send(c); // Ilmoittaa annetun arvon terminaalissa
            is_fraction = 1;
        }
        else if (c == 'd')
        {
            uart_send_str("\r\n");
            break;
        }
    }
    return result;
}
