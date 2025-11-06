/**
 ******************************************************************************
 * @file    state_manager.c
 * @brief   Implementation of string-based system state manager.
 *
 * @details
 * Uses readable state names ("IDLE", "CONFIG", "MODULATION") instead of numeric
 * codes. Each state transition triggers a corresponding handler defined in
 * state_functions.c.
 ******************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include "bitwise.h"
#include "uart_utils.h"
#include "state_functions.h"
#include "state_manager.h"

static const char *current_state = "IDLE";

void state_manager_init(void)
{
    // Configure button (PC13 input, pull-up)
    bits_val(GPIOC->MODER, 2, 13, 0); // input
    bits_val(GPIOC->PUPDR, 2, 13, 1); // pull-up

    current_state = "IDLE";
    handle_state_idle();  // call first state function

    printf("[INIT] Starting in %s state\r\n", current_state);
}

// Get current state string
const char *state_manager_get_state(void)
{
    return current_state;
}

// Set state programmatically
void state_manager_set_state(const char *new_state)
{
    if (strcmp(new_state, current_state) == 0)
        return; // no change

    current_state = new_state;

    // Run appropriate handler
    if (strcmp(current_state, "IDLE") == 0)
    {
        handle_state_idle();
    }
    else if (strcmp(current_state, "CONFIG") == 0)
    {
        handle_state_config();
    }
    else if (strcmp(current_state, "MODULATION") == 0)
    {
        handle_state_modulation();
    }
    else
    {
        printf("[ERROR] Unknown state: %s\r\n", current_state);
        return;
    }

    printf("[STATE] Switched to %s\r\n", current_state);
}

// This is called in main loop to check if button is pressed and update state
void state_manager_update(void)
{
    // Button active low (pressed = 0)
    int button_pressed = !(GPIOC->IDR & (1 << 13));

    if (button_pressed)
    {
        // IDLE → CONFIG → MODULATION → IDLE
        if (strcmp(current_state, "IDLE") == 0)
            state_manager_set_state("CONFIG");
        else if (strcmp(current_state, "CONFIG") == 0)
            state_manager_set_state("MODULATION");
        else
            state_manager_set_state("IDLE");

        // simple delay to avoid multiple triggers
        for (volatile int d = 0; d < 100000; d++);
    }
}
