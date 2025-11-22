/* 
Controls the different states of the system: IDLE, CONFIG and ACTIVE
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
    print("--- IDLE MODE ---");
    print("PWM is OFF. Press button to Config.");
    
    // Turn off PWM (Disable Counter)
    TIM2->CR1 &= ~TIM_CR1_CEN;
    
    // Set Duty Cycle to 0
    TIM2->CCR1 = 0;
}

void handle_config_state(void) {
    print("--- CONFIG MODE ---");
    bits_val(GPIOA->MODER, 2, 5, 1); // Set PA5 to Output mode for LED indication
    
    // Stop timer (just to be sure)
    TIM2->CR1 &= ~TIM_CR1_CEN;
    
    // Set pin high (LED on) to indicate Config Mode
    GPIOA->BSRR = (1 << 5);
    print("Entering CONFIGURATION state");

    // Get Kp parameter
    print("Enter Kp (0-9): ");
    param_Kp = (float)(uart_recv() - '0');
    print("Kp set to: %d", (int)param_Kp);

    // Get Ki parameter
    print("Enter Ki (0-9): ");
    param_Ki = (float)(uart_recv() - '0');
    print("Ki set to: %d", (int)param_Ki);

    // Get Reference Voltage
    print("Give a value for reference voltage (0-9):");
    param_Ref = (float)(uart_recv() - '0');

    // give some terminal output
    print("Given parameter for reference voltage: %d", (int)param_Ref);

    // control timer prescaler (LED blinking speed)
    // FIX: Explicitly cast float param_Ref to int before shifting
    int shift_amount = (int)param_Ref - 1;
    
    // Safety check to prevent negative shift
    if (shift_amount < 0) shift_amount = 0; 
    
    TIM2->PSC = 512 << shift_amount;
    
    print("Parameters are now set. Please change the mode!");
}

void handle_active_state(void) {
    // 1. ENSURE PWM IS ON (Safety check)
    if (!(TIM2->CR1 & TIM_CR1_CEN)) {
        bits_val(GPIOA->MODER, 2, 5, 2); // Ensure PA5 is AF (PWM)
        TIM2->CR1 |= TIM_CR1_CEN;        // Enable Timer
    }

    // 2. STATIC STATE (Must persist across function calls)
    static PIState_t piState = {0}; 
    
    // 3. PRINTING TIMER
    static uint32_t last_print_time = 0;
    uint32_t current_time = GetTick(); 

    // 4. CONTROL LOOP (Runs every single time function is called - Fast)
    PIParams_t params;
    params.Kp = param_Kp;
    params.Ki = param_Ki;
    params.reference = param_Ref;
    params.outputMin = 0.0f;
    params.outputMax = 10.0f; // Max voltage

    // Run PI Algorithm
    // dt = 0.01s (You should ideally measure actual delta time)
    float control_signal = PIController_Update(&params, &piState, 0.01f);

    // Log data every second
    if ((current_time - last_print_time) >= 1000) 
    {

        // Run Converter Model
        float converter_output = runConverterModel(control_signal);
        char buffer[80]; 

        print("[ACTIVE] Running...");
        
        // Print voltages and parameters
        print("Reference/target voltage: %.2f V | Output voltage: %.2f V | Control signal: %.2f", 
              param_Ref, converter_output, control_signal);
        print("Params -> Kp: %.2f, Ki: %.2f", param_Kp, param_Ki);
        print("");

        last_print_time = current_time;
    }
}