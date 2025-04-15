#ifndef DEBUG_FUNC_H
#define DEBUG_FUNC_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <arm_neon.h>

#include "debug_func.h"
#include "generate.h"
#include "encoder.h"
#include "modulate.h"
#include "decode.h"
#include "monitor.h"

// print functions for arrays of different types
void print_array (uint8_t* array, size_t size);
void print_array8 (int8_t* array, size_t size);
void print_array_32 (int32_t* array, size_t size);
void print_array_float (float* array, size_t size);
void display_uint8x16 (uint8x16_t vector);
void display_int8x16 (int8x16_t vector);
void display_int16x8 (int16x8_t vector);
void display_int32x4 (int32x4_t vector);

// functional test
int main();


#endif