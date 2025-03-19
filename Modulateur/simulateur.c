#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define K 4
#define REPS 3

// print functions for arrays of different types
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
void encoder_repetition_encode(const uint8_t* UK, uint8_t* CN, size_t k, size_t repetitions) {
    for (int n=0; n<k*repetitions; n++) 
        CN[n] = UK[n%k] ;
}

// modulates encoded codeword where 0 -> 1, 1 -> -1
// read from C_N, write into X_N
void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t N) {
    for (; N>0; N--)
        XN[N-1] = (CN[N-1]?-1:1);
}

void agwn(const int32_t* XN, float* YN, size_t N)) {
	// passer de EB/N0 à ES/N0 -> need coderate
	// Calculer sigma à partir de là
	// Générer un N de la loi normale pour l'ajouter au X et on a le Y
	//
    for (; N>0; N--) {
	float n = 0;
	YN[N] = XN[N] + n;
    }
}

void demodulate (float* YN, float* LN, size_t N) {
        memcpy (LN, YN, N*sizeof(float));
}

void decode_hard (float* LN, uint8_t* UO, size_t N, size_t K) {
    int8_t hard_decision[N];
    // Reduce float to corresponding int (-1 ; 1)
    for (; N>0; N--) {
        hard_decision[N-1] = (LN[N-1] >= 0 ? 1 : -1);
    }

    // Average out the hard decisions by summing them
    int8_t average[K] = {0};
    int nb_reps = N/K;
    for (int i=0; i<K; K++) {
	for (int j= 0; j<nb_reps; j++) average[i] += hard_decision[i + j*K] ;
    }

    // Map hard decision to decoded message
    for (int i = 0; i<K; i++) 
	UO[i] = (average[K]>=0?0:1);

}

void decode_soft () 

int main() {

    int N = K*REPS;
    uint8_t UK[K];
    uint8_t CN[N];
    int32_t XN[N];
    //Init random
    srand(time(NULL));   // Initialization, should only be called once.

    // test
    for (int i=0; i<20; i++) {
        source_generate(UK, K);
        printf("\nTableau genere : ");
        print_array(UK, K);
        encoder_repetition_encode(UK,CN,K,REPS);
        printf("\nTableau encode : ");
        print_array(CN,N);
        module_bpsk_modulate(CN, XN, N);
        printf("\nTableau module : ");
        print_array_32(XN, N);
        printf("\n__________\n");
    }

    return 0;
}
