#include "debug_func.h"

// print functions for arrays of different types

void print_array_binary(void const * const ptr, size_t const size)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    printf("[ ");
    for (i = 0; i < size; i++) {
        printf("\n\t");
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u ; ", byte);
        }
    }
    printf("]");
}

void print_array (uint8_t* array, size_t size) {
    printf("[ ");
    for (int i=0; i<size; i++) {
        if (i%8 == 0 && i!=0) printf("\n");
        printf("%d ; ", array[i]);
    }
    printf("]");
}
void print_array8 (int8_t* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%d ; ", array[i]);
    }
    printf("]");
}
void print_array_32 (int32_t* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        if (i%8 == 0) printf("\n");
        printf("%d ; ", array[i]);
    }
    printf("]");
}
void print_array_float (float* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%f ; ", array[i]);
    }
    printf("]\n");
}
void display_uint8x16 (uint8x16_t vector) {
    uint8_t T[16];
    printf("[");
    vst1q_u8(T, vector);
    for (int i=0; i<16; i++) {
        printf("%i ; ", T[i]);
    }
    printf("]\n");
}
void display_int8x16 (int8x16_t vector) {
    int8_t T[16];
    printf("[");
    vst1q_s8(T, vector);
    for (int i=0; i<16; i++) {
        printf("%i ; ", T[i]);
    }
    printf("]\n");
}

void display_int16x8 (int16x8_t vector) {
    int16_t T[8];
    printf("[");
    vst1q_s16(T, vector);
    for (int i=0; i<8; i++) {
        printf("%i ; ", T[i]);
    }
    printf("]\n");
}

void display_int32x4 (int32x4_t vector) {
    int32_t T[4];
    printf("[");
    vst1q_s32(T, vector);
    for (int i=0; i<4; i++) {
        printf("%i ; ", T[i]);
    }
    printf("]\n");
}

int main() {
 
    //Init random
    srand(time(NULL));   // Initialization, should only be called once.
    const gsl_rng_type * rangentype;
    rangentype = gsl_rng_default;
    gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 

    size_t K = 32, N = 64, REPS = 2;
    uint8_t UK[K/8], CN[N/8], PN[K/8];
    int32_t XN[N];
    float YN[N], LN[N];
    int8_t L8N[N];
    uint8_t VN[K];

    for (int i = 0; i < 10; i++)
    {
        // float sigma=0;
        //  Generate message
        //source_generate(UK, K);
        source_gen_bit_pack(UK,K/8);
         printf("\nTableau genere : ");
         print_array(UK, K/8);
         printf("\nbinary : ");
         print_array_binary(UK, K/8);
         printf("\n");

        // Encoded message
        //encoder_repetition_encode(UK, CN, K, REPS);
        encoder_rep_encode_bit_pack(UK,CN,K/8,REPS);
         printf("\nTableau encode : ");
         print_array(CN, N/8);
         printf("\nbinary : ");
         print_array_binary(CN, N/8);
         printf("\n");

        // Modulated message
        //module_bpsk_modulate(CN, XN, N);
        module_bpsk_modulate_bit_unpack(CN, XN, N);
        printf("\nTableau module : ");
        print_array_32(XN, N);
        printf("\n");

        // Modulated message
        //module_bpsk_modulate_neon(CN, XN, N);
        //printf("\nTableau module neon : \n");
        //print_array_32(XN, N);
        //printf("\n__________\n");

        // Canal message
        channel_AGWN_add_noise(XN, YN, N, 0.1, rangen);
        // printf("Tableau transmis : \n");
        // print_array_float(YN, N);
        // printf("\n");

        // Demodulated message
        modem_BPSK_demodulate(YN, LN, N, 0.1);
        // printf("Tableau demodule : \n");
        // print_array_float(LN, N);
        // printf("\n");

        // convert to fixed point
        // quantizer_transform8(LN, L8N, N, 5, 3);

        // printf("Tableau hard dec : \n");
        // codec_repetition_hard_decode8_neon(L8N, VN, K, REPS);
        // printf("\n");

        // printf("Tableau soft dec NORMAL: \n");
         codec_repetition_soft_decode(LN, VN, K, REPS);
         printf("\nTableau decode : ");
         print_array(VN,K);
         printf("\n");


        bit_packer(VN, PN, K/8);
        printf("\nTableau repacked : ");
         print_array(PN, K/8);
        printf("\n__________\n");

        // printf("Tableau soft dec w SIMD: \n");
        // codec_repetition_soft_decode8_neon(L8N, VN, K, REPS);
        // printf("\n");

        // printf("Tableau soft dec no SIMD: \n");
        // codec_repetition_soft_decode8(L8N, VN, K, REPS);
        // printf("\n");

    }

    gsl_rng_free(rangen);
    return 0;
}