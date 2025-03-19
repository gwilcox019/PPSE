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

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t N, float sigma){
        memcpy (LN, YN, N*n_reps*sizeof(float));
}

void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t K, size_t n_reps) {
    int8_t N = K*n_reps;
    int8_t hard_decision[N];
    // Reduce float to corresponding int (-1 ; 1)
    for (; N>0; N--) {
        hard_decision[N-1] = (LN[N-1] >= 0 ? 1 : -1);
    }

    // Average out the hard decisions by summing them
    int8_t average[K] = {0}; 
    for (int i=0; i<K; K++) {
	for (int j= 0; j<n_reps; j++) average[i] += hard_decision[i + j*K] ;
    }

    // Map hard decision to decoded message
    for (int i = 0; i<K; i++) 
	UO[i] = (average[K]>=0?0:1);

}

void codec_repetition_soft_decode (const float* L_N, uint8_t *V_N, size_t K, size_t n_reps) {

}

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t K, uint64_t) *n_bit_errors, uint64_t *n_frame_errors) {
    int flag = 0;
    for (; K>0; K--) {
	if (U_K[K] != V_K[K]) {
	    if (!flag) { *n_frame_errors++; flag++ };
	    n_bit_errors++;
	}

    }

}

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
