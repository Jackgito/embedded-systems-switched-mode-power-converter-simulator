#include <bitwise.h>
#include <stm32f4xx.h>
#include <string.h>
#include "uart_utils.h"                     // UART communication functions
#include "stateController/state_manager.h"  // State machine management
#include "converterModel/converter_model.h" // Converter simulation model
#include "terminalUI/terminal_ui.h"         // Terminal user interface
#include "piController/piController.h"      // PI controller module

// System configuration parameters
static struct {
    float reference_voltage;  // Target output voltage
    float proportional_gain;  // PI controller P term
    float integral_gain;      // PI controller I term
} system_config = {
    .reference_voltage = 5.0f,
    .proportional_gain = 1.0f,
    .integral_gain = 0.1f
};

// PI controller structures
static PIParams_t piParams;
static PIState_t  piState;

extern uint32_t SystemCoreClock; // System clock frequency

int main(void)
{
    // --- Peripheral Initializations ---

    // Enable GPIOA & GPIOC clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    // Enable USART2 clock & timer 2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN | RCC_APB1ENR_TIM2EN;

    // --- UART (USART2) Initialization ---
    uart_init();

    // --- LED (LD2) and Timer (TIM2) Initialization ---
    // Configure PA5 (LED) as TIM2_CH1 alternate function
    bits_val(GPIOA->AFR[0], 4, 5, 1); // PA5 -> TIM2_CH1

    // Configure timer 2 for PWM output @ 1kHz
    TIM2->PSC   = 3;                 // Prescaler (84MHz / (3+1) = 21MHz)
    TIM2->ARR   = 20999;             // Auto-reload (21MHz / 21000 = 1kHz)
    bits_val(TIM2->CCMR1, 4, 1, 7);  // CH1 PWM mode 1
    bits_val(TIM2->CCMR1, 4, 5, 3);  // Preload enable
    TIM2->CCER |= TIM_CCER_CC1E;     // Enable CH1 output
    TIM2->CCR1 = 0;                  // Start with LED off
    TIM2->EGR = TIM_EGR_UG;          // Update generation to apply settings
    TIM2->CR1 |= TIM_CR1_CEN;        // Start timer

    // --- BUTTON (B1) Initialization ---
    bits_val(GPIOC->MODER, 2, 13, 0); // PC13 Mode = Input
    bits_val(GPIOC->PUPDR, 2, 13, 1); // Pull-up

    // Initialize state machine and UI
    state_manager_init();
    terminal_ui_init();

    // --- Initialize PI Controller ---
    PIController_Init(&piParams, &piState);
    piParams.reference = system_config.reference_voltage;
    piParams.Kp = system_config.proportional_gain;
    piParams.Ki = system_config.integral_gain;
    piParams.outputMin = 0.0f;
    piParams.outputMax = 1.0f;

    // Initial system state
    state_manager_set_state("IDLE");

    // --- Main loop ---
    while (1)
    {
        // Update state machine (handles button presses, UART commands)
        state_manager_update();

        const char* current_state = state_manager_get_state();

        if (strcmp(current_state, "IDLE") == 0)
        {
            // IDLE: system off
            TIM2->CCR1 = 0; // LED off
            //PIController_Reset(&piState);
        }
        else if (strcmp(current_state, "CONFIG") == 0)
        {
            // Configuration mode - user adjusts gains/reference via UART
            terminal_ui_update();

            // Update controller parameters dynamically
            piParams.Kp = system_config.proportional_gain;
            piParams.Ki = system_config.integral_gain;
            piParams.reference = system_config.reference_voltage;

            // Blink LED to indicate config mode
            static uint32_t last_tick = 0;
            if ((HAL_GetTick() - last_tick) > 500) {
                last_tick = HAL_GetTick();
                TIM2->CCR1 = (TIM2->CCR1 > 0) ? 0 : (TIM2->ARR / 4);
            }
        }
        else if (strcmp(current_state, "MODULATION") == 0)
        {
            static uint32_t last_control_update = 0;
            static float current_brightness = 0.0f;

            // Update control loop every 10 ms (100 Hz)
            if ((HAL_GetTick() - last_control_update) > 10) {
                last_control_update = HAL_GetTick();

                // Run converter model and PI controller
                float output_voltage = ConverterModel_Step(0.0f);
                current_brightness = PIController_Update(&piParams, &piState, output_voltage);

                // Clamp to safe range [0,1]
                if (current_brightness > 1.0f) current_brightness = 1.0f;
                if (current_brightness < 0.0f) current_brightness = 0.0f;

                // Apply PWM duty cycle
                TIM2->CCR1 = (uint32_t)(current_brightness * TIM2->ARR);
            }
        }

        // Small non-blocking delay (polling-friendly)
        for (volatile int i = 0; i < 1000; i++);
    }
}
