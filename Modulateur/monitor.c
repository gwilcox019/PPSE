#include "monitor.h"

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors) {
    int flag = 0;
    for (int i=0; i<k; i++) {
        if (U_K[i] != V_K[i]) {
            if (!flag) { 
                *n_frame_errors = *n_frame_errors+1; 
                flag++; 
            }
            *n_bit_errors = *n_bit_errors+1;
        }
    }
}