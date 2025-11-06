/**
 ******************************************************************************
 * @file    terminal_ui.c
 * @brief   UART-based user interface for interactive configuration.
 *
 * @details
 * This module provides a text-based interface over UART for:
 *   - Changing the operating mode (IDLE, CONFIG, MODULATION)
 *   - Setting PI controller gains (proportional, integral)
 *   - Setting reference output voltage
 *
 * All interaction occurs through the serial terminal using printf().
 ******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stm32f4xx.h>
#include "terminal_ui.h"
#include "stateController/state_manager.h"
#include "converterModel/converter_model.h"

#define INPUT_BUFFER_SIZE 64

// --- Local variables ---
static char input_buffer[INPUT_BUFFER_SIZE];
static int buffer_index = 0;

// --- Local helper function ---
static void handle_command(const char *cmd);

// Initialization
void terminal_ui_init(void)
{
    printf("\r\n=====================================================\r\n");
    printf("   STM32 Terminal Control Interface\r\n");
    printf("=====================================================\r\n");
    printf("Available commands:\r\n");
    printf("  mode=IDLE | CONFIG | MODULATION\r\n");
    printf("  proportional_gain=<float>\r\n");
    printf("  integral_gain=<float>\r\n");
    printf("  reference_voltage=<float>\r\n");
    printf("-----------------------------------------------------\r\n");
    printf("Example:\r\n");
    printf("  mode=CONFIG\r\n");
    printf("  proportional_gain=1.5\r\n");
    printf("  reference_voltage=12.0\r\n");
    printf("-----------------------------------------------------\r\n\r\n");
    printf(">> ");
}

// Called continuously from main loop to process UART input
void terminal_ui_update(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        char c = USART2->DR;

        if (c == '\r' || c == '\n')  // end of command
        {
            input_buffer[buffer_index] = '\0';

            if (buffer_index > 0)
            {
                printf("\r\n");
                handle_command(input_buffer);
                buffer_index = 0;
            }

            printf(">> ");
        }
        else if (isprint((unsigned char)c) && buffer_index < INPUT_BUFFER_SIZE - 1)
        {
            input_buffer[buffer_index++] = c;
            printf("%c", c); // echo character
        }
    }
}

// Command parsing and execution
static void handle_command(const char *cmd)
{
    if (strncmp(cmd, "mode=", 5) == 0)
    {
        const char *mode = cmd + 5;
        state_manager_set_state(mode);
    }
    else if (strncmp(cmd, "proportional_gain=", 18) == 0)
    {
        float value = atof(cmd + 18);
        converter_model_set_proportional_gain(value);
        printf("[CONFIG] Proportional gain updated to %.3f\r\n", value);
    }
    else if (strncmp(cmd, "integral_gain=", 14) == 0)
    {
        float value = atof(cmd + 14);
        converter_model_set_integral_gain(value);
        printf("[CONFIG] Integral gain updated to %.3f\r\n", value);
    }
    else if (strncmp(cmd, "reference_voltage=", 18) == 0)
    {
        float value = atof(cmd + 18);
        converter_model_set_reference_voltage(value);
        printf("[MODULATION] Reference voltage set to %.3f V\r\n", value);
    }
    else
    {
        printf("[ERROR] Unknown command: %s\r\n", cmd);
    }
}
