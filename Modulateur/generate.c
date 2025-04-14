#include "generate.h"

void source_generate(uint8_t* UK, size_t k) {
    for (; k>0; k--)
        UK[k-1] = rand()%2 ;      // Returns a pseudo-random integer between 0 and RAND_MAX.
}

void source_gen_bit_pack(uint8_t* UK, size_t k) {
    for (; k>0; k--) UK[k-1] = rand(); 
}

void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    for (; K>0; K--)
        U_K[K-1] = 0 ; 
}