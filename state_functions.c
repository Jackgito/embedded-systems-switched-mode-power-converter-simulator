/* Controls the different states of the system: IDLE, CONFIG and ACTIVE
*/

#include "state_functions.h"
#include "uart_utils.h"
#include "piController.h"
#include "converter_model.h"
#include <stm32f4xx.h>
#include <stdio.h>
#include <bitwise.h> 

// --- Global Parameters ---
float param_Kp = 1.0f;
float param_Ki = 0.1f;
float param_Ref = 5.0f;

// Enables loop
extern uint32_t GetTick(void);

void handle_idle_state(void) {
    println("--- IDLE MODE ---");
    
    // Turn off PWM (Disable Counter)
    TIM2->CR1 &= ~TIM_CR1_CEN;
    // Set Duty Cycle to 0
    TIM2->CCR1 = 0;
}

void handle_config_state(void) {
    println("--- CONFIG MODE ---");
    bits_val(GPIOA->MODER, 2, 5, 1); // Output mode
    TIM2->CR1 &= ~TIM_CR1_CEN;
    GPIOA->BSRR = (1 << 5); // LED ON
    println("Entering CONFIGURATION state");

    // Get all parameters first
    print("Enter Kp (0.01 - 10.0): ", false);
    param_Kp = read_float_from_uart(0.01f, 10.0f);
    
    print("Enter Ki (0.001 - 1.0): ", false);
    param_Ki = read_float_from_uart(0.001f, 1.0f);

    print("Enter reference voltage (0.1 - 15.0V): ", false);
    param_Ref = read_float_from_uart(0.1f, 15.0f);

    // Display all selected values at once
    println("\r\n--- CONFIGURATION SUMMARY ---");
    println("Kp: %.4f", param_Kp);
    println("Ki: %.4f", param_Ki);
    println("Reference Voltage: %.2fV", param_Ref);

    int shift_amount = (int)param_Ref - 1;
    if (shift_amount < 0) shift_amount = 0; 
    if (shift_amount > 15) shift_amount = 15; 
    
    TIM2->PSC = 512 << shift_amount;
    
    println("Parameters set. Press the button to start the modulation.");
}

void handle_active_state(void) {

    // "static" ensures these keep their value between function calls.
    static PIState_t piState = {0}; 
    static float current_feedback_voltage = 0.0f; 
    static uint32_t last_step_time = 0;
    static uint32_t last_print_time = 0;
    static uint8_t is_first_run = 1; 

    // If the PWM was disabled (we came from IDLE/CONFIG), reset the model & controller.
    if (TIM2->CCR1 == 0 && is_first_run) {
        
        // Reset the entire struct to zero. 
        piState = (PIState_t){0};
        
        current_feedback_voltage = 0.0f;
        is_first_run = 0; // Clear flag so we don't reset while running
    }

    uint32_t current_time = GetTick(); 

    // CONTROL LOOP 
    // Run every 20ms (visualization speed)
    if ((current_time - last_step_time) >= 20) {
        
        // --- A. Setup Controller Parameters ---
        PIParams_t params;
        params.Kp = param_Kp;
        params.Ki = param_Ki;
        params.reference = param_Ref;
        params.outputMin = 0.0f;
        params.outputMax = 80.0f; 

        // Run PI Controller
        float control_signal = PIController_Update(&params, &piState, current_feedback_voltage);

        // Run Converter Simulation
        float new_voltage = runConverterModel(control_signal);
        current_feedback_voltage = new_voltage;

        // Update PWM (LED Brightness)
        uint32_t period = TIM2->ARR; 

        // Calculate LED brightness
        // In handle_active_state() function, find the section that updates the PWM:
        // Replace the duty cycle calculation with:
        float max_voltage = 80.0f;
        float led_brightness = current_feedback_voltage / max_voltage;

        // Clamp brightness between 0 and 1
        if (led_brightness > 1.0f) led_brightness = 1.0f;
        if (led_brightness < 0.0f) led_brightness = 0.0f;

        // Update PWM
        TIM2->CCR1 = (uint32_t) (led_brightness * TIM2->ARR);

        last_step_time = current_time;

        // Print input and output voltages
        if ((current_time - last_print_time) >= 1000) {
            println("Target voltage: %.2fV | Measured voltage: %.2fV", 
                    param_Ref, current_feedback_voltage);
            last_print_time = current_time;
        }
    }
}