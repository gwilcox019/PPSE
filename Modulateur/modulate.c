#include "modulate.h"

void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n--)
        XN[n-1] = (CN[n-1]?-1:1);
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

// adds random noise following a normal distribution
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen) {
    for (; n>0; n--) {
        float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
        Y_N[n-1] = X_N[n-1] + v;
    }

}