#include "state_functions.h"
#include "uart_utils.h"  // Tarvitaan uart_send_str() varten
#include "bitwise.h"     // Tarvitaan bits_val() varten
#include <stm32f4xx.h>   // Tarvitaan GPIOA ja TIM2 rekistereihin pääsyyn

void handle_state_1(void)
{
	// Tila 1: LED PÄÄLLÄ
	// Määritä PA5 GPO-tilaan
	bits_val(GPIOA->MODER, 2, 5, 1);
	// Pysäytä ajastin (varmuuden vuoksi)
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Aseta pinni korkeaksi (LED päälle)
	GPIOA->BSRR = (1 << 5);
	uart_send_str("Vaihdettu tilaan 1: LED Paalla\r\n");
	// receive single character from UART
	// and convert it to an integer, see
	// the ASCII character code table
	char c = uart_recv() - '0';

	// allow integers 1-4
	//if ((c < 1) || (c > 4))
		//continue;

	// control timer prescaler (LED blinking speed)
	TIM2->PSC = 512 << (c - 1);

	// give some terminal output
	uart_send_str("You pressed: ");
	uart_send(c + '0');
	uart_send_str("\r\n");
}

void handle_state_2(void)
{
	// Tila 2: LED POIS PÄÄLTÄ
	// Määritä PA5 GPO-tilaan
	bits_val(GPIOA->MODER, 2, 5, 1);
	// Pysäytä ajastin (varmuuden vuoksi)
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Aseta pinni matalaksi (LED pois)
	GPIOA->BSRR = (1 << (5 + 16)); // Käytä BSRR-rekisterin reset-osaa
	uart_send_str("Vaihdettu tilaan 2: LED Pois Paalta\r\n");
}

void handle_state_3(void)
{
	// Tila 3: LED VILKKUU
	// Määritä PA5 Alternate Function -tilaan
	bits_val(GPIOA->MODER, 2, 5, 2);
	// Käynnistä ajastin
	TIM2->CR1 |= TIM_CR1_CEN;
	uart_send_str("Vaihdettu tilaan 3: LED Vilkkuu\r\n");
}
