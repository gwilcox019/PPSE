#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include <stdio.h>
#include <arm_neon.h>
#include <pthread.h>
//Check errors by comparing original message U_K with decoded message V_K.
//Modifies n_bit_errors and n_frame_errors.
void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors);

//Same but uses neon SIMD instructions
void monitor_neon (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors);
#endif
