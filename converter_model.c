/**
 * This implements a state-space model of a 3-stage DC-DC converter.
 * The model uses a discrete-time state-space representation to simulate
 * the behavior of the power electronics converter.
 * 
 * Implementation based on the state-space model from:
 * https://moodle.lut.fi/pluginfile.php/2665570/mod_resource/content/10/basics_of_digital_control_embedded_2020.pdf (slide 29)
 */

#include "converter_model.h"
#include "uart_utils.h"
#include <stdio.h>
#include <stdint.h>

float runConverterModel(float u_in) {
    static ConverterState state = {0};
    
    // Perform the conversion (math only)
    float output = convert(u_in, &state);
  
    return output;
}


float convert(float u_in, ConverterState* state) {
    // Definitions of temporary variables for the next state (k+h)
    float i1_k_plus_h, u1_k_plus_h, i2_k_plus_h, u2_k_plus_h, i3_k_plus_h, u3_k_plus_h;

    // Calculating the next state (k+h) using Eq 3 from instructions
    i1_k_plus_h = (0.9652f * state->i1_k) + (-0.0172f * state->u1_k) + (0.0057f * state->i2_k) +
                  (-0.0058f * state->u2_k) + (0.0052f * state->i3_k) + (-0.0251f * state->u3_k) +
                  (0.0471f * u_in);

    u1_k_plus_h = (0.7732f * state->i1_k) + (0.1252f * state->u1_k) + (0.2315f * state->i2_k) +
                  (0.07f * state->u2_k) + (0.1282f * state->i3_k) + (0.7754f * state->u3_k) +
                  (0.0377f * u_in);

    i2_k_plus_h = (0.8278f * state->i1_k) + (-0.7522f * state->u1_k) + (-0.0956f * state->i2_k) +
                  (0.3299f * state->u2_k) + (-0.4855f * state->i3_k) + (0.3915f * state->u3_k) +
                  (0.0404f * u_in);

    u2_k_plus_h = (0.9948f * state->i1_k) + (0.2655f * state->u1_k) + (-0.3848f * state->i2_k) +
                  (0.4212f * state->u2_k) + (0.3927f * state->i3_k) + (0.2899f * state->u3_k) +
                  (0.0485f * u_in);

    i3_k_plus_h = (0.7648f * state->i1_k) + (-0.4165f * state->u1_k) + (-0.4855f * state->i2_k) +
                  (-0.3366f * state->u2_k) + (-0.0986f * state->i3_k) + (0.7281f * state->u3_k) +
                  (0.0373f * u_in);

    u3_k_plus_h = (1.1056f * state->i1_k) + (0.7587f * state->u1_k) + (0.1179f * state->i2_k) +
                  (0.0748f * state->u2_k) + (-0.2192f * state->i3_k) + (0.1491f * state->u3_k) +
                  (0.0539f * u_in);

    // Update state
    state->i1_k = i1_k_plus_h;
    state->u1_k = u1_k_plus_h;
    state->i2_k = i2_k_plus_h;
    state->u2_k = u2_k_plus_h;
    state->i3_k = i3_k_plus_h;
    state->u3_k = u3_k_plus_h;

    return u3_k_plus_h;
}

// Helper function kept for debugging
void display_results(float u_in, float u_out) {
    println("--- Converter Model ---");
    println("Input: %.4f V", u_in);
    println("Output: %.4f V", u_out);
}