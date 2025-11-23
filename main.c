#include <bitwise.h>
#include <stm32f4xx.h>
#include "uart_utils.h"
#include "state_functions.h" 
#include "piController.h"    
#include <stdio.h>

// Timer setup for enabling 1 second loop
volatile uint32_t msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;
}

uint32_t GetTick(void) {
    return msTicks;
}

void SysTick_Init(void) {
    SysTick_Config(SystemCoreClock / 1000);
}


// Reads the user button (PC13)
// return 1 if button was just pressed, 0 otherwise.

int check_button_press(void) {
    static int prev_state = 1; // 1 = Unpressed (Pull-up)
    int current_state = (GPIOC->IDR & (1 << 13)) ? 1 : 0;
    int pressed = 0;

    // Detect if button was just pressed
    if (prev_state == 1 && current_state == 0) {
        pressed = 1;
        // Debounce to avoid double clicks
        for(volatile int i=0; i<50000; i++); 
    }
    
    prev_state = current_state;
    return pressed;
}

typedef enum {
    STATE_IDLE = 1,
    STATE_CONFIG,
    STATE_ACTIVE
} SystemState_t;

int main()
{
    SysTick_Init(); 

    // Enable Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN | RCC_APB1ENR_TIM2EN;

    uart_init();
    uart_send_str("\r\n--- DC-DC Power converter started ---\r\n");

    // Timer 2 Setup (PWM) - 50kHz frequency
    bits_val(GPIOA->AFR[0], 4, 5, 1); // PA5 -> TIM2_CH1
    TIM2->PSC = 0;                    // No prescaler
    TIM2->ARR = 319;                  // For 16MHz clock: 16,000,000 / (1 * 320) = 50kHz
    bits_val(TIM2->CCMR1, 4, 1, 7);   // PWM mode 1
    TIM2->CCER |= TIM_CCER_CC1E;      // Enable channel 1
    TIM2->CCR1 = 0;                   // Start with 0 duty

    // Button Setup (PC13)
    bits_val(GPIOC->MODER, 2, 13, 0); // Input
    bits_val(GPIOC->PUPDR, 2, 13, 1); // Pull-up

    SystemState_t state = STATE_IDLE;
    int is_new_state = 1; 

    while (1) {

        if (check_button_press()) {
            // Determine the NEXT state based on CURRENT state
            switch (state) {
                case STATE_IDLE:   state = STATE_CONFIG; break;
                case STATE_CONFIG: state = STATE_ACTIVE; break;
                case STATE_ACTIVE: state = STATE_IDLE;   break;
            }
            // Flag that we have just entered a new state
            is_new_state = 1; 
        }

        switch (state) {
            case STATE_IDLE:
                // RUN ONCE
                if (is_new_state) {
                    handle_idle_state();
                    is_new_state = 0; // Task done, wait for next transition
                }
                break;

            case STATE_CONFIG:
                // RUN ONCE
                if (is_new_state) {
                    handle_config_state();
                    is_new_state = 0; // Task done
                }
                break;

            case STATE_ACTIVE:
                // RUN REPEATEDLY
                handle_active_state(); 
                break;
        }
    }
}