#include "state_functions.h"
#include "uart_utils.h"  // Tarvitaan uart_send_str() varten
#include "bitwise.h"     // Tarvitaan bits_val() varten
#include <stm32f4xx.h>   // Tarvitaan GPIOA ja TIM2 rekistereihin pääsyyn
#include "pi_controller.h"   // Liitetään PI-säätimen kirjasto (PI_Init varten)


volatile float param_Kp = 0.0f;
volatile float param_Ki = 0.0f;

// Tila 1: Idle
void handle_state_1(void)
{

	uart_send_str("\r\nChanged to state 1: Idle\r\n");

	// Määritä PA5 GPO-tilaan
	bits_val(GPIOA->MODER, 2, 5, 1);


	// Pysäytä ajastin
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Aseta pinni matalaksi (LED pois)
	GPIOA->BSRR = (1 << (5 + 16)); // Käytä BSRR-rekisterin reset-osaa


}

// Tila 2: Configuration
void handle_state_2(void)
{

	uart_send_str("\r\nChanged to state 2: Configuration\r\n");

	// Määritä PA5 GPO-tilaan
	bits_val(GPIOA->MODER, 2, 5, 1);
	// Pysäytä ajastin
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Aseta LED päälle
	GPIOA->BSRR = (1 << 5);

	/////////////////////////////////////////////////////////////////////////////

	// --- Kp (Float) ---
	uart_send_str("Give Kp, end with 'd': ");
	param_Kp = uart_recv_float_until_d();
	uart_send_str("Kp set. \r\n");

	// --- Ki (Float) ---
	uart_send_str("Give Ki, end with 'd': ");
	param_Ki = uart_recv_float_until_d();
	uart_send_str("Ki set. \r\n");

	////////////////////////////////////////////////////////////////////////////


	uart_send_str("\r\nAll parameters updated.\r\n");

}

void handle_state_3(void)
{
	// Tila 3: LED VILKKUU
	uart_send_str("\r\nChanged to mode 3: Modulation\r\n");

	PI_Init();

	// Ledin ohjauksen määritykset harkasta 5
	// Määritä PA5 Alternate Function -tilaan (PWM - ohjausta varten)
	bits_val(GPIOA->MODER, 2, 5, 2);

	// Määritä mikä Alternate Function: AF1 on TIM2_CH1
	bits_val(GPIOA->AFR[0], 4, 5, 1);
	// Käynnistä ajastin
	TIM2->CR1 |= TIM_CR1_CEN;
}
