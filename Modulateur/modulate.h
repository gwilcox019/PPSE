#ifndef MODULATE_H
#define MODULATE_H

#include <stdint.h>
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n);

void modem_BPSK_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N);

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma);

void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen);

#endif 