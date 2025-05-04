#ifndef MODULATE_H
#define MODULATE_H

#include <stdint.h>
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <arm_neon.h>

// modulates encoded codeword where 0 -> 1, 1 -> -1
// read from C_N, write into X_N
void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n);

// using bit packing
void module_bpsk_modulate_bit_unpack (const uint8_t* CN, int32_t* XN, size_t n);

// using SIMD
void module_bpsk_modulate_neon (const uint8_t* CN, int32_t* XN, size_t n);

// Always outputs 1s
void modem_BPSK_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N);

//Multiplies by a factor 2/sigma² to ignore sigma values
void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma);

// using SIMD
void modem_BPSK_demodulate_neon (const float* Y_N, float* L_N, size_t n, float sigma);

//Adds noise to value
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen);

#endif 