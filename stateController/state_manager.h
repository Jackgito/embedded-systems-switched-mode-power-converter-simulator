/**
 ******************************************************************************
 * @file    state_manager.h
 * @brief   Simple string-based state machine for STM32 Nucleo.
 *
 * @details
 * Manages system modes identified by descriptive strings:
 *   - "IDLE"
 *   - "CONFIG"
 *   - "MODULATION"
 *
 * The active state can be changed with the user button (PC13) or
 * programmatically via state_manager_set_state().
 ******************************************************************************
 */

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>

// Initialize hardware and starting state
void state_manager_init(void);

// Called repeatedly inside the main loop
void state_manager_update(void);

// Change state programmatically (optional, for UART or control logic)
void state_manager_set_state(const char *new_state);

// Get current state string
const char *state_manager_get_state(void);

#endif
