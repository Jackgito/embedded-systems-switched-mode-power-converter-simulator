/**
 * @file converter_model.h
 * @brief Header file for the switched mode power converter model
 *
 * This file contains the declaration of the converter model function.
 * The model implements a state-space representation of a DC-DC converter.
 */

#ifndef CONVERTER_MODEL_H
#define CONVERTER_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Simulates one time step of the converter model
 *
 * @param u_in Input voltage to the converter
 * @return float Output voltage of the converter (u3)
 */
float runConverterModel(float u_in);

#ifdef __cplusplus
}
#endif

#endif /* CONVERTER_MODEL_H */
