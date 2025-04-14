#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdio.h>

// encodes frame of k bits by repeating it
// read from the buffer U_K, write into C_N
void encoder_repetition_encode(const uint8_t* UK, uint8_t* CN, size_t k, size_t repetitions);

// encodes for bit packing
void encoder_rep_encode_bit_pack(const uint8_t* UK, uint8_t* CN, size_t k, size_t reps);

#endif