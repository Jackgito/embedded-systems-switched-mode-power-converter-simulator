#include <bitwise.h>
#include <stm32f4xx.h>
#include "uart_utils.h"       // Tuodaan UART-funktiot
#include "state_functions.h"  // Tuodaan tilakoneen funktiot
#include "piController.h"
#include "converter_model.h"

// Define system states
typedef enum {
    STATE_IDLE = 1,    // System is idle
    STATE_CONFIG,      // Configuration mode
    STATE_ACTIVE       // Active control mode
} SystemState_t;

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

	// --- Display welcome message ---
	uart_send_str("\r\n");
	uart_send_str("*******************************************\r\n");
	uart_send_str("*        STM32 PI Controller App         *\r\n");
	uart_send_str("*  Version: 1.0                          *\r\n");
	uart_send_str("*                                         *\r\n");
	uart_send_str("*  Description:                           *\r\n");
	uart_send_str("*  This application implements a PI      *\r\n");
	uart_send_str("*  controller for motor/speed control    *\r\n");
	uart_send_str("*  using STM32 Nucleo F411RE board.      *\r\n");
	uart_send_str("*******************************************\r\n\r\n");
	uart_send_str("System initialized. Starting main loop...\r\n");

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

	// --- State machine variables ---
	SystemState_t state = STATE_IDLE; // Start in IDLE state
	int button_was_pressed = 0; // Button state tracking (debounce)

	// Initialize PI Controller
	PIParams_t piParams;
	PIState_t piState;
	PIController_Init(&piParams, &piState);

	// Set initial state (IDLE: LED On)
	state = STATE_IDLE;
	handle_idle_state();

	// --- Päälooppi ---
	while (1)
	{
		// Tarkista napin tila
		int button_is_pressed = !(GPIOC->IDR & (1 << 13));

		// Jos nappi on juuri painettu
		if (button_is_pressed && !button_was_pressed)
		{
			// Cycle through states: IDLE -> CONFIG -> ACTIVE -> IDLE...
			switch (state) {
				case STATE_IDLE:
					state = STATE_CONFIG;
					break;
				case STATE_CONFIG:
					state = STATE_ACTIVE;
					break;
				case STATE_ACTIVE:
				default:
					state = STATE_IDLE;
					break;
			}

			// Execute actions based on the new state
			switch (state) {
				case STATE_IDLE:
					handle_idle_state();
					uart_send_str("State: IDLE\r\n");
					break;

				case STATE_CONFIG:
					handle_config_state();
					uart_send_str("State: CONFIG\r\n");
					break;

				case STATE_ACTIVE:
					handle_active_state();
					uart_send_str("State: ACTIVE\r\n");
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
