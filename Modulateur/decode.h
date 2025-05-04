#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>
#include <stdio.h>
#include <arm_neon.h>
#include <math.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//Basic versions
void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t k, size_t n_reps);

void codec_repetition_soft_decode (const float* L_N, uint8_t *V_N, size_t k, size_t n_reps);

// Bit packer
void bit_packer (const uint8_t* V_N, uint8_t* P_N, size_t k);

//Convert to fixed-point
void quantizer_transform8 (const float* L_N, int8_t* L8_N, size_t N, size_t s, size_t f);

//Fixed-point versions
void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);

void codec_repetition_soft_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);

//Neon versions
void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);

void codec_repetition_soft_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps);

#endif