/**
 ******************************************************************************
 * @file    terminal_ui.h
 * @brief   Human-readable UART terminal interface for STM32.
 *
 * @details
 * Allows the user to control system parameters and modes using
 * descriptive text commands such as:
 *   - mode=IDLE / CONFIG / MODULATION
 *   - proportional_gain=<float>
 *   - integral_gain=<float>
 *   - reference_voltage=<float>
 *
 * The interface provides feedback through printf() over UART.
 ******************************************************************************
 */

#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <stdint.h>

// Initializes the UART-based terminal interface
void terminal_ui_init(void);

// Continuously checks for user input and executes valid commands
void terminal_ui_update(void);

#endif
