// This code follows the example given in the following slideset (slide 29)
// https://moodle.lut.fi/pluginfile.php/2665570/mod_resource/content/10/basics_of_digital_control_embedded_2020.pdf

#include "converter_model.h"


// x(k+1) = A * x(k) + B * u(k)
// y(k) = C * x(k)

float runConverterModel(float u_in) {
    // Store the current state (k). Initialized to 0.0
    static float i1_k = 0.0;
    static float u1_k = 0.0;
    static float i2_k = 0.0;
    static float u2_k = 0.0;
    static float i3_k = 0.0;
    static float u3_k = 0.0;

    // Definitions of temporary variables for the next state (k+h)
    float i1_k_plus_h;
    float u1_k_plus_h;
    float i2_k_plus_h;
    float u2_k_plus_h;
    float i3_k_plus_h;
    float u3_k_plus_h;

    // Calculating the next state (k+h)
    // This is the matrix multiplication x(k+h) = A*x(k) + B*u_in(k)

    // i1(k+h)
    i1_k_plus_h = (0.9652 * i1_k) + (-0.0172 * u1_k) + (0.0057 * i2_k) +
                  (-0.0058 * u2_k) + (0.0052 * i3_k) + (-0.0251 * u3_k) +
                  (0.0471 * u_in);

    // u1(k+h)
    u1_k_plus_h = (0.7732 * i1_k) + (0.1252 * u1_k) + (0.2315 * i2_k) +
                  (0.07 * u2_k) + (0.1282 * i3_k) + (0.7754 * u3_k) +
                  (0.0377 * u_in);

    // i2(k+h)
    i2_k_plus_h = (0.8278 * i1_k) + (-0.7522 * u1_k) + (-0.0956 * i2_k) +
                  (0.3299 * u2_k) + (-0.4855 * i3_k) + (0.3915 * u3_k) +
                  (0.0404 * u_in);

    // u2(k+h)
    u2_k_plus_h = (0.9948 * i1_k) + (0.2655 * u1_k) + (-0.3848 * i2_k) +
                  (0.4212 * u2_k) + (0.3927 * i3_k) + (0.2899 * u3_k) +
                  (0.0485 * u_in);

    // i3(k+h)
    i3_k_plus_h = (0.7648 * i1_k) + (-0.4165 * u1_k) + (-0.4855 * i2_k) +
                  (-0.3366 * u2_k) + (-0.0986 * i3_k) + (0.7281 * u3_k) +
                  (0.0373 * u_in);

    // u3(k+h)
    u3_k_plus_h = (1.1056 * i1_k) + (0.7587 * u1_k) + (0.1179 * i2_k) +
                  (0.0748 * u2_k) + (-0.2192 * i3_k) + (0.1491 * u3_k) +
                  (0.0539 * u_in);


    // Updating the state variables for the next function call
    // (k+h) becomes (k) for the next iteration
    i1_k = i1_k_plus_h;
    u1_k = u1_k_plus_h;
    i2_k = i2_k_plus_h;
    u2_k = u2_k_plus_h;
    i3_k = i3_k_plus_h;
    u3_k = u3_k_plus_h;

    // Returning the output y = u3
    return u3_k;
}
