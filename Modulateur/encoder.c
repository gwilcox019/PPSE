#include "encoder.h"

// encodes frame of k bits by repeating it
// read from the buffer U_K, write into C_N
void encoder_repetition_encode(const uint8_t* UK, uint8_t* CN, size_t k, size_t repetitions) {
    for (int n=0; n<k*repetitions; n++) 
        CN[n] = UK[n%k] ;
}

void encoder_rep_encode_bit_pack(const uint8_t* UK, uint8_t* CN, size_t k, size_t reps) {
    int n;
    for (int i=0; i<reps; i++) {
        n = i*k;
        for (int j=0; j<k; j++) {
            CN[n+j] = UK[j];
        }
    }
}