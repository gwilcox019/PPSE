#include "decode.h"

void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t k, size_t n_reps) {
    // Reduce float to corresponding int (-1 ; 1)
    // Then average out the hard decisions by summing them
    int average = 0;
    int tie =0;
    for (int i=0; i<k; i++) {
        average = 0;
	    for (int j= 0; j<n_reps; j++) {
            average += (L_N[i+j*k] < 0 ? -1  : 1) ;
        }
        // Map hard decision to decoded message
	if (average < 0) V_N[i] = 1;
	else if (average > 0) V_N[i] = 0;
	else {
		V_N[i] = tie;
		tie = 1 - tie;
	}
    }
}

// converts decoded back to packed bit -> call this after decode for bit packing
void bit_packer (const uint8_t* V_N, uint8_t* P_N, size_t k) {
    for (int i=0; i<k; i++) { 
        P_N[i] = 0;
        for (int j=7; j>=0; j--) {
            P_N[i] |= (V_N[8*i+7-j] << j); // left shift unpacked element to correct spot and then | with result
        }
    }
}

void codec_repetition_soft_decode (const float* L_N, uint8_t *V_N, size_t k, size_t n_reps) {
    float avg;
    for (int i=0; i<k; i++) {
        avg = 0;
        for (int j=0; j<n_reps; j++) {
            avg += L_N[j*k+i];
        }
        if (avg < 0) V_N[i] = 1;
        else V_N[i] = 0;
    }
}

// transform to fixed point
void quantizer_transform8 (const float* L_N, int8_t* L8_N, size_t N, size_t s, size_t f) {
    float range = pow(2,s-1);
    float frange = pow(2,f);
    for (int i = 0; i<N; i++) {
        float tmp = MIN(MAX(round(frange*L_N[i]),-range+1),range-1);
        L8_N[i] = (int) tmp;

        //We do not want to send a 0 if it is not a 0, because 0 will be inferred as a positive value always
        if (L8_N[i] == 0 && L_N[i] < 0) L8_N[i] = -1;
        else if (L8_N[i] == 0 && L_N[i] > 0) L8_N[i] = 1;
    }
}

void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    int average = 0;
    int tie = 0;
    for (int i=0; i<K; i++) {
        average = 0;
	    for (int j= 0; j<n_reps; j++) {  
            average += (L8_N[i+j*K] < 0 ? -1  : 1);
        }

        //Tie-breaker to avoid favorizing 0
        if (average < 0) V_K[i] = 1;
        else if (average > 0) V_K[i] = 0;
        else {           
            V_K[i] = tie;
            tie = 1 - tie;
        }
    }
}

void codec_repetition_soft_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    int avg;
    for (int i=0; i<K; i++) {
        avg = 0;
        for (int j=0; j<n_reps; j++) {
                avg += ((int)(L8_N[j*K+i]));
        }
        if (avg < 0) V_K[i] = 1;
        else V_K[i] = 0;
    }
}

void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    // truc intermédiaire pour charger la valeur
    int array_nb = K/16;
    int8x16_t decomposed; //Stores the raw data that we are working on
    int8x16_t sum; //Stores the local sum for that part of the array
    int8x16_t only1 = vdupq_n_s8(1);
    for (int a=0; a<array_nb; a++) { //For each group of 16 values
        sum = vdupq_n_s8(0);
        for (int r=0; r<n_reps; r++) { //For each repetition of the codeword
            //Préparation de la partie a
            decomposed = vld1q_s8(L8_N + (r*K + a*16));
            decomposed = (int8x16_t)vcltzq_s8(decomposed); //below 0 -> -1 / above 0 -> 0
            decomposed = vaddq_s8 (decomposed, decomposed); //We get back -2s (below 0) and 0s (above 0)
            decomposed = vaddq_s8 (decomposed, only1); //We get back -1 (below 0) and +1 (above 0))

            // Adding vectors to get average
            sum = vqaddq_s8(decomposed, sum);
        }

        //Deciding based on average
        sum = (int8x16_t)vcltzq_s8(sum); //below 0 -> -1 / above 0 -> 0
        sum = vandq_s8 (sum, only1); // below 0 -> 1 / above 0 -> 0

        //Storing result
        vst1q_s8((int8_t*)V_K+(a*16), sum);
    }   
}

void codec_repetition_soft_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
   // K = message size, always multiple of 16
    int k16 = K/16; // number of 16-wide chunks in message
    int cw; // number of 16-wide chunks already treated
    int8x16_t l8; // temp vector for loading array
    int8x16_t avg[k16]; // avg vectors
    for (int i=0; i<k16; i++) avg[i] = vdupq_n_s8(0); // initialize zeros
    // calc avg 
    for (int n=0; n<n_reps; n++) {
        cw = n*16*k16;
        for (int i=0; i<k16; i++) {
           l8 = vld1q_s8(L8_N + cw + i*16); // load next 16 ints from L8_N
           avg[i] = vqaddq_s8(l8, avg[i]); // cumulative avg
        }
    } 
    int8x16_t one = vdupq_n_s8(1);
    for (int i=0; i<k16; i++) {
	    int8x16_t tmp = (int8x16_t)vcltzq_s8(avg[i]);
        avg[i] = vandq_s8(tmp, one); // check avg < 0, use bit mask bc true result = 0xFFFF_FFFF
        vst1q_s8((int8_t*)V_K+i*16, avg[i]); // save result
    }
}  