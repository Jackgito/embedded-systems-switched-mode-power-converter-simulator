#include "piController.h"

/**
 * @brief Initialize PI controller parameters and state.
 */
void PIController_Init(PIParams_t *params, PIState_t *state)
{
    if (!params || !state) return;

    // Default values (can be overridden by user later)
    params->Kp = 0.5f;
    params->Ki = 0.1f;
    params->reference = 1.0f;
    params->outputMin = 0.0f;
    params->outputMax = 1.0f;

    state->integral = 0.0f;
    state->lastError = 0.0f;
}

/**
 * @brief Compute PI controller output.
 * @param params Pointer to PI parameters structure
 * @param state Pointer to PI state structure
 * @param feedback Current measured system output
 * @return Control signal (normalized, typically 0.0–1.0)
 */
float PIController_Update(PIParams_t *params, PIState_t *state, float feedback)
{
    if (!params || !state) return 0.0f;

    // Compute error
    float error = params->reference - feedback;

    // Update integral term
    state->integral += error * params->Ki;

    // Camp integrator within output range
    if (state->integral > params->outputMax)
        state->integral = params->outputMax;
    else if (state->integral < params->outputMin)
        state->integral = params->outputMin;

    // Compute raw output
    float output = params->Kp * error + state->integral;

    // Clamp output to valid range
    if (output > params->outputMax)
        output = params->outputMax;
    else if (output < params->outputMin)
        output = params->outputMin;

    // Store last error
    state->lastError = error;

    return output;
}

void PIController_Reset(PIState_t *state)
{
    if (!state) return;
    state->integral = 0.0f;
    state->lastError = 0.0f;
}
