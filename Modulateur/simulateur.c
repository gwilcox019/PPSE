#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define K 4
#define REPS 3

void print_array (uint8_t* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%d ; ", array[i]);
    }
    printf("]");
}
void print_array_32 (int32_t* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%d ; ", array[i]);
    }
    printf("]");
}

// generates random frame of k bits
//write into the buffer U_K
void source_generate(uint8_t* UK, size_t k) {
    for (; k>0; k--)
        UK[k-1] = rand()%2 ;      // Returns a pseudo-random integer between 0 and RAND_MAX.
}

// encodes frame of k bits by repeating it
// read from the buffer U_K, write into C_N
void encoder_repetition_encode(const uint8_t* UK, uint8_t* CN, size_t repetitions) {
    for (int r=0; r<repetitions; r++) {
        for (int k=0; k<sizeof(UK); k++) {
            CN[r+k] = UK[k];
        }
    }
}

// modulates encoded codeword where 0 -> 1, 1 -> -1
// read from C_N, write into X_N
void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t N) {
    for (; N>0; N--)
        XN[N-1] = (CN[N-1]?-1:1);
}



int main() {
    uint8_t UK[K];
    int32_t xn_tmp[K];
    int N = K*REPS;
    uint8_t CN[N];
    int32_t XN[N];
    //Init random
    srand(time(NULL));   // Initialization, should only be called once.

    
    for (int i=0; i<20; i++) {
        source_generate(UK, K);
        printf("\nTableau généré : ");
        print_array(UK, K);
        encoder_repetition_encode(UK,CN,REPS);
        printf("\nTableau encodé : ");
        print_array(CN,N);
        printf("\nTableau modulé : ");
        module_bpsk_modulate(UK, xn_tmp, K);
        print_array_32(xn_tmp, K);
        printf("\n__________\n");
    }

    return 0;
}