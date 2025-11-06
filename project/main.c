#include <bitwise.h>
#include <stm32f4xx.h>
#include "uart_utils.h"       // Tuodaan UART-funktiot
#include "state_functions.h"  // Tuodaan tilakoneen funktiot

extern uint32_t SystemCoreClock; // system clock frequency

int main()
{
	// --- Alustukset ---

	// enable GPIOA & GPIOC clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

	// enable USART2 clock & timer 2 clock
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN | RCC_APB1ENR_TIM2EN;

	// --- UART (USART2) Alustus ---
	uart_init(); // Kutsutaan alustusfunktiota

	// --- LED (LD2) ja Ajastin (TIM2) Alustus ---
	// configure PA5 (LED) as timer 2 CH1 (Tämä on AF-tila vilkkumista varten)
	bits_val(GPIOA->AFR[0], 4, 5, 1); // PA5 -> TIM2_CH1
	// (MODER-rekisteriä hallinnoidaan nyt dynaamisesti state_functions.c-tiedostossa)

	// configure timer 2 (compare mode)
	TIM2->PSC   = 512;               // prescaler
	TIM2->ARR   = 3999;              // auto-reload
	bits_val(TIM2->CCMR1, 4, 1, 7);  // CH1 PWM mode
	TIM2->CCER |= TIM_CCER_CC1E;     // CH1 capture/compare enable
	TIM2->CCR1 = TIM2->ARR/2;        // set 50% duty cycle

	// --- NAPPI (B1) Alustus ---
	bits_val(GPIOC->MODER, 2, 13, 0); // PC13 Mode = Input (00)
	bits_val(GPIOC->PUPDR, 2, 13, 1); // PC13 PUPDR = Pull-up (01)

	// --- Tilakoneen muuttujat ---
	int state = 1; // Aloitetaan tilasta 1
	int button_was_pressed = 0; // Lippu napin tilan seurantaan (debounce)

	// Asetetaan alkutila (Tila 1: LED Päällä)
	handle_state_1();

	// --- Päälooppi ---
	while (1)
	{
		// Tarkista napin tila
		int button_is_pressed = !(GPIOC->IDR & (1 << 13));

		// Jos nappi on juuri painettu
		if (button_is_pressed && !button_was_pressed)
		{
			// Vaihda tilaa (1 -> 2 -> 3 -> 1 ...)
			state++;
			if (state > 3) {
				state = 1;
			}

			// Suorita toiminnot uuden tilan perusteella kutsumalla funktiota
			switch (state)
			{
				case 1:
					handle_state_1();
					break;

				case 2:
					handle_state_2();
					break;

				case 3:
					handle_state_3();
					break;
			}

			button_was_pressed = 1;
		}
		// Jos nappi vapautetaan
		else if (!button_is_pressed)
		{
			button_was_pressed = 0;
		}
	}
}
