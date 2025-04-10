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
	if (L8_N[i] == 0 && L_N[i] < 0) L8_N[i] = -1;
	else if (L8_N[i] == 0 && L_N[i] > 0) L8_N[i] = 1;
if (L8_N[i] < 0 && L_N[i] > 0) printf("womp"); else if (L8_N[i] > 0 && L_N[i] < 0) printf("sadge");
    }
   // sleep(10);
}

void codec_repetition_hard_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    int average = 0;
    int tie = 0;
    for (int i=0; i<K; i++) {
        average = 0;
	    for (int j= 0; j<n_reps; j++) {  
            average += (L8_N[i+j*K] < 0 ? -1  : 1);
        }
        if (average < 0) V_K[i] = 1;
        else if (average > 0) V_K[i] = 0;
        else {           
            V_K[i] = tie;
            tie = 1 - tie;
        }
        //V_K[i] = (average<0?1:0);
    }
}

void codec_repetition_soft_decode8(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    int avg;
//    printf("appel à soft8\n");
    for (int i=0; i<K; i++) {
        avg = 0;
        for (int j=0; j<n_reps; j++) {
            if (avg != 0x7FFF && avg != 0x8000) { //Once overflow is reached, we cannot undo it
                avg += ((int)(L8_N[j*K+i]));
	        }
        }
        if (avg < 0) V_K[i] = 1;
        else V_K[i] = 0;
    }
}



void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    // truc intermédiaire pour charger la valeur
    int array_nb = K/16;
    //print_array8(L8_N, K*n_reps);
    int8x16_t decomposed; //Stores the raw data that we are working on
    int8x16_t sum; //Stores the local sum for that part of the array
    int8x16_t only1 = vdupq_n_s8(1);
    for (int a=0; a<array_nb; a++) { //Pour chaque groupe de 16 valeurs (taille de K)
        sum = vdupq_n_s8(0);
        for (int r=0; r<n_reps; r++) { //Pour chacune des répétitions
            //Préparation de la partie a
            decomposed = vld1q_s8(L8_N + (r*K + a*16));
            decomposed = (int8x16_t)vcltzq_s8(decomposed); //below 0 -> -1 / above 0 -> 0
            decomposed = vaddq_s8 (decomposed, decomposed); //On récupère des 0 (above 0) et des -2 (below 0)
            decomposed = vaddq_s8 (decomposed, only1); //On récupère -1 si on avait moins de 0 et +1 si on avait plus de 0

            // Addition des vecteurs pour avoir la moyenne
            sum = vqaddq_s8(decomposed, sum);
        }

        //Décision à partir de la moyenne
        sum = (int8x16_t)vcltzq_s8(sum); //On récupère 0 si on avait + que 0 à la somme, -1 si on avait - que 0
        sum = vandq_s8 (sum, only1); // On a 1 si la somme était négative et 0 si la somme était positive

        //Stockage du résultat
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