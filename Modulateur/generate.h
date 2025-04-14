#ifndef GENERATE_H
#define GENERATE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// generates random frame of k bits
//write into the buffer U_K
void source_generate(uint8_t* UK, size_t k);

// generates for bit packing
void source_gen_bit_pack(uint8_t* UK, size_t k);

// alternative generator with all zero
void source_generate_all_zeros(uint8_t *U_K, size_t K);

#endif