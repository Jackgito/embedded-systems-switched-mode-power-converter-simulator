#ifndef CONVERTER_MODEL_H
#define CONVERTER_MODEL_H

#include <stdint.h>

// Struct Definition
typedef struct {
    float i1_k;
    float u1_k;
    float i2_k;
    float u2_k;
    float i3_k;
    float u3_k;
} ConverterState;

// Public Functions
float runConverterModel(float u_in);
float convert(float u_in, ConverterState* state);
void display_results(float u_in, float u_out);

#endif