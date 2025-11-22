#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Parameters for the PI controller.
 * These are typically configured by the user through buttons or UART.
 */
typedef struct {
    float Kp;          // Proportional gain
    float Ki;          // Integral gain
    float reference;   // Desired output (setpoint)
    float outputMin;   // Minimum control output (e.g. 0.0)
    float outputMax;   // Maximum control output (e.g. 1.0)
} PIParams_t;

/**
 * @brief Internal state variables of the PI controller.
 */
typedef struct {
    float integral;     // Integrator accumulation
    float lastError;    // Optional: used for derivative or diagnostics
} PIState_t;

/**
 * @brief Initialize the PI controller with default parameters.
 * @param params Pointer to parameter struct.
 * @param state Pointer to controller state struct.
 */
void PIController_Init(PIParams_t *params, PIState_t *state);

/**
 * @brief Execute one iteration of the PI control algorithm.
 * @param params Pointer to parameter struct.
 * @param state Pointer to controller state struct.
 * @param feedback Current measured system output.
 * @return Control signal (normalized, typically 0.0–1.0)
 */
float PIController_Update(PIParams_t *params, PIState_t *state, float feedback);

/**
 * @brief Reset the controller state (integrator, error).
 * Useful when switching modes (e.g. IDLE → MODULATE).
 */
void PIController_Reset(PIState_t *state);

#endif // PI_CONTROLLER_H
