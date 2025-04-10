#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include <stdio.h>

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors);

#endif