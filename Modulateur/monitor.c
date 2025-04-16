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

void monitor_neon (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors) {
    // Comparer U_K à V_K
    // Boucle sur le nombre de parties dans le tableau
    // Addition réduction du nombre de différences

    int nb_parts = k/16;
    int8x16_t only1 = vdupq_n_s8(1);
    char flag=0;

    for (int i=0; i<nb_parts; i++) {
        // Load both arrays
        uint8x16_t u = vld1q_u8(U_K + (i * 16));
        uint8x16_t v = vld1q_u8(V_K + (i * 16));

        //Compare - if equal, -1 / if different, 0
        int8x16_t res = (int8x16_t) vceqq_u8(u,v);

        // If equal, 0 / if different, 1 - we add 1
        res = vaddq_s8(res, only1);
        // Accumulate total number of differences
        int8_t acc = vaddvq_s8(res);

        // Adding a frame error if not already detected + we detect one now
        if (!flag && acc != 0) {
            *n_frame_errors = *n_frame_errors+1; 
            flag++; //We won't flag another one
        } 

        // Adding nb of errors to bit error
        *n_bit_errors += acc;
    }
}
