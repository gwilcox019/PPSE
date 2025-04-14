#include "modulate.h"

void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n--)
        XN[n-1] = (CN[n-1]?-1:1);
}

void module_bpsk_modulate_neon (const uint8_t* CN, int32_t* XN, size_t n) {
    printf("modulating\n");
    int8x16_t l8; // temp vector for loading array
    int8x16_t one = vdupq_n_s8(1); // bit mask 
    for (int i=0; i<n; i+=16) {
        l8 = vld1q_s8((int8_t*)CN+i); // load next 16 values
        l8 = (int8x16_t)vcltzq_s8(l8); // compare < 0
        vst1q_s8((int8_t*)XN+i,vandq_s8(l8,one)); // mask to get 1 or 0, store in XN
    }
}

void modem_BPSK_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N) {
    for (; N>0; N--)
        X_N[N-1] = 1;
}

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma){
    float constant = 2.0f/(sigma*sigma);
    for (int i=0; i<n; i++) {
        L_N[i] = Y_N[i] * constant; 
    }
}

void modem_BPSK_demodulate_neon (const float* Y_N, float* L_N, size_t n, float sigma) {
    float32x4_t tmp, cst;
    float constant = 2.0f/(sigma*sigma);
    cst = vdupq_n_f32(constant); 
    for (int i=0; i<n; i+=4) {
        tmp = vld1q_f32(Y_N+i); 
        vst1q_f32(L_N+i,vmulq_f32(cst,tmp));
    }
}


// adds random noise following a normal distribution
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen) {
    for (; n>0; n--) {
        float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
        Y_N[n-1] = X_N[n-1] + v;
    }

}