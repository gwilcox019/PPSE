#include "generate.h"

// generates random frame of k bits
//write into the buffer U_K
void source_generate(uint8_t* UK, size_t k) {
    for (; k>0; k--)
        UK[k-1] = rand()%2 ;      // Returns a pseudo-random integer between 0 and RAND_MAX.
}

// alternative generator with all zero
void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    for (; K>0; K--)
        U_K[K-1] = 0 ; 
}