#ifndef DEBUG_FUNC_H
#define DEBUG_FUNC_H

#include <stdint.h>
#include <stdio.h>
#include <arm_neon.h>

// print functions for arrays of different types
void print_array (uint8_t* array, size_t size);
void print_array8 (int8_t* array, size_t size);
void print_array_32 (int32_t* array, size_t size);
void print_array_float (float* array, size_t size);
void display_uint8x16 (uint8x16_t vector);
void display_int8x16 (int8x16_t vector);


#endif