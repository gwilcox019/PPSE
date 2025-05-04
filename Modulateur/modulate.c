#include "modulate.h"

void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n--)
        XN[n-1] = (CN[n-1]?-1:1);
}

void module_bpsk_modulate_bit_unpack (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n=n-8) { // for each element of CN
        for (int i=0; i<8; i++) { // for each bit of this element of CN
            XN[-1-i] = ((CN[n/8-1] & (1<<i)) ? -1:1); // check bit i of CN
        }
    } 
}

// 0 -> 1, 1 -> -1
void module_bpsk_modulate_neon (const uint8_t* CN, int32_t* XN, size_t n) {
    int8x16_t l8; // temp vector for loading array
    int16x8_t s16[2];
    int32x4_t s32[4]; // temp vector for storing array
    int8x16_t one = vdupq_n_s8(1); // bit mask 
    for (int i=0; i<n; i+=16) {
        // modulate
        l8 = vld1q_s8((int8_t*)CN+i); // load next 16 values
        l8 = (int8x16_t)vcgtzq_s8(l8);  // check if > 0, 1 -> -1, 0 -> 0
        l8 = vaddq_s8(l8,l8); // 2*l8, 1 -> -2, 0 -> 0
        l8 = vaddq_s8(l8,one); // +1, 1 -> -1, 0 -> 1

        // convert to 32 bit
        s16[0] = vmovl_s8(vget_low_s8(l8));
        s16[1] = vmovl_high_s8(l8); // convert to 16 bit
        s32[0] = vmovl_s16(vget_low_s16(s16[0]));
        s32[1] = vmovl_high_s16(s16[0]);
        s32[2] = vmovl_s16(vget_low_s16(s16[1]));
        s32[3] = vmovl_high_s16(s16[1]);

        // store result
        for (int j=0; j<4; j++) vst1q_s32(XN+i+4*j,s32[j]); 
    }
}

void module_bpsk_modulate_bit_pack (const uint8_t* CN, int32_t* XN, size_t n, size_t N) {
    // n is length of bit packed codeword, N is length of modulated 
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
