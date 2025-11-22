#include "state_functions.h"
#include "uart_utils.h"  // For UART functions
#include "bitwise.h"     // For bits_val()
#include "piController.h" // For PI controller definitions
#include <stm32f4xx.h>   // For STM32 register access

volatile uint8_t param_Kp = 0;
volatile uint8_t param_Ki = 0;
volatile uint8_t param_Ref = 0;

void handle_idle_state(void)
{
	// State 1: Idle Mode
	// Configure PA5 as GPO
	bits_val(GPIOA->MODER, 2, 5, 1);
	// Stop timer (just to be sure)
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Set pin low (LED off)
	GPIOA->BSRR = (1 << (5 + 16)); // Using BSRR reset part
	uart_send_str("\r\nEntering IDLE state\r\n");
}

void handle_config_state(void)
{
	// State 2: Configuration Mode
	// Configure PA5 as GPO
	bits_val(GPIOA->MODER, 2, 5, 1);
	// Stop timer (just to be sure)
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Set pin high (LED on)
	GPIOA->BSRR = (1 << 5);
	uart_send_str("\r\nEntering CONFIGURATION state\r\n");

	// Get Kp parameter
	uart_send_str("\r\nEnter Kp (0-9): ");
	param_Kp = uart_recv() - '0';
	uart_send_str("\r\nKp set to: ");
	uart_send(param_Kp + '0');

	// Get Ki parameter
	uart_send_str("\r\nEnter Ki (0-9): ");
	param_Ki = uart_recv() - '0';
	uart_send_str("\r\nKi set to: ");
	uart_send(param_Ki + '0');
	uart_send_str("\r\n");
	/////////////////////////////////////////////////////////////////////////////
	uart_send_str("Give a value for reference voltage:");
	param_Ref = uart_recv() - '0';


	// give some terminal output
	uart_send_str("\r\nGiven parameter for reference voltage: ");
	uart_send(param_Ref + '0');
	uart_send_str("\r\n");

	// control timer prescaler (LED blinking speed)
	TIM2->PSC = 512 << (param_Ref - 1);
	/////////////////////////////////////////////////////////////////////////////
	uart_send_str("\r\nParameters are now set. Please change the mode!\r\n");
}

void handle_active_state(void)
{
	// State 3: Active Control Mode
	// Configure PA5 as Alternate Function (PWM)
	bits_val(GPIOA->MODER, 2, 5, 2);
	// Start timer for PWM
	TIM2->CR1 |= TIM_CR1_CEN;
	
	// Update PI controller with current parameters
	PIParams_t params = {
	    .Kp = (float)param_Kp,
	    .Ki = (float)param_Ki,
	    .reference = (float)param_Ref,
	    .outputMin = 0.0f,
	    .outputMax = 1.0f
	};
	PIState_t state = {0};
	PIController_Update(&params, &state, 0);
	
	uart_send_str("\r\nEntering ACTIVE control mode\r\n");
	uart_send_str("Current parameters - ");
	uart_send_str("Kp: ");
	uart_send(param_Kp + '0');
	uart_send_str(", Ki: ");
	uart_send(param_Ki + '0');
	uart_send_str(", Ref: ");
	uart_send(param_Ref + '0');
	uart_send_str("\r\n");
}
