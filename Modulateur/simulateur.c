#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <gsl/gsl_rng.h>
// #include <gsl/gsl_randist.h>
#include <getopt.h>


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

void print_array_float (float* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%f ; ", array[i]);
    }
    printf("]\n");
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
void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n--)
        XN[n-1] = (CN[n-1]?-1:1);
}

// adds random noise following a normal distribution
// void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma) {
//     const gsl_rng_type * rangentype;
//     rangentype = gsl_rng_default;
//     gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 
//     for (; n>0; n--) {
//         float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
//         Y_N[n-1] = X_N[n-1] + v;
//     }
//     gsl_rng_free (rangen);
// }

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma){
        memcpy (L_N, Y_N, n*sizeof(float));
}

void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t k, size_t n_reps) {
    // Reduce float to corresponding int (-1 ; 1)
    // Then average out the hard decisions by summing them
    int average = 0;
    for (int i=0; i<k; i++) {
        average = 0;
	    for (int j= 0; j<n_reps; j++) {
            average += (L_N[i+j*k] >= 0 ? 1 : -1) ;
            printf("adding %i to average\n", (L_N[i+j*k] >= 0 ? 1 : -1));
        }
        printf("Average is %i\n", average);
        // Map hard decision to decoded message
        V_N[i] = (average>=0?0:1);
    }
}

// 12 elems dans tableau, k = 4, reps = 3
// Elements à additionner : 0-4-8-12 / 1-5-9-13 / 2-6-10-14 / 3-7-11-15
// on parcourt k
//

void codec_repetition_soft_decode (const float* L_N, uint8_t *V_N, size_t k, size_t n_reps) {

}

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors) {
    int flag = 0;
    for (; k>0; k--) {
        if (U_K[k] != V_K[k]) {
            if (!flag) { 
                *n_frame_errors = *n_frame_errors+1; 
                flag++; 
            }
            *n_bit_errors = *n_bit_errors+1;
        }
    }
}



int main( int argc, char** argv) {
    // // simulation parameters
    // float min_SNR = 0;
    // float max_SNR = 12;
    // float step_val = 1;
    // uint32_t f_max = 100;
    // uint32_t info_bits = 32;
    // uint32_t codeword_size = 128;
    // void (*decoder_fn) (const float*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode; 
    // char filepath[11] = {0};

    // // We use getopt to parse command line arguments
    // // An option is a parameter beginning with '-' (different from "-" and "--")
    // // getopt (argc, argv, format)
    // // Format is a string containing all valid options (as letters) to be parsed
    // // If one of those chars is followed by a ':', it means it expects an argument after
    // // It is the case for all of our arguments
    // // The following argument is then stored in optarg variable
    // int opt;
    // //Loops while something to read
    // while ((opt = getopt(argc, argv, "m:M:s:e:K:N:D:f:")) != -1) {
    //     switch(opt) {
    //         case 'm': min_SNR = atof(optarg); break;
    //         case 'M': if ((max_SNR = atof(optarg)) == 0) max_SNR = 12; break;
    //         case 's': if ((step_val = atof(optarg)) == 0) step_val = 1; break;
    //         case 'e': if ((f_max = atoi(optarg)) == 0) f_max = 100; break;
    //         case 'K': if ((info_bits = atoi(optarg)) == 0) info_bits = 32; break;
    //         case 'N': if ((codeword_size = atoi(optarg)) == 0) codeword_size = 128; break;
    //         case 'D':
    //             if (strcmp(optarg, "rep-hard") == 0) decoder_fn = codec_repetition_hard_decode;
    //             break;
    //         case 'f': sprintf(filepath, "sim_%i.csv", atoi(optarg)); break;
                
    //     }
    // }

    // // Arrays & simulation parameters
    // uint8_t U_K[info_bits]; // Source message
    // uint8_t C_N[codeword_size]; // Repetition coded message
    // int32_t X_N[codeword_size]; // Modulated message
    // float Y_N[codeword_size];  // Received message after canal
    // float L_N[codeword_size];  // Demodulated message
    // uint8_t V_K[info_bits];// Decoded message

    // uint64_t n_bit_errors, n_frame_errors, n_frame_simulated; // Frame and bit stats
    // float sigma; // Variance
    // float SNR_better; // Es/N0 instead of Eb/N0
    // float R = info_bits/codeword_size; // Ratio
    // uint32_t n_reps = codeword_size/info_bits; //Number of repetitions

    // // Stats - computed after one loop
    // float ber, fer;

    // // Output file
    // FILE* file;
    // if (filepath[0] == 0) file = stdout;
    // else file = fopen(filepath, "w");
    // fprintf(file, "# Bit Errors, # Frame Errors, # Simulated frames, BER, FER, Time for this SNR, Average time for one frame\n");

    // // Time computation
    // clock_t start_time, end_time;
    // float elapsed=0;
    // float average=0;
    
    // //Init random
    // srand(time(NULL));   // Initialization, should only be called once.

    // for (float val = min_SNR; val <= max_SNR; val+=step_val) {
    //     start_time = clock();
    //     n_bit_errors = 0;
    //     n_frame_errors = 0;
    //     n_frame_simulated = 0;

    //     SNR_better = val + 10*log(R*1);
    //     sigma = sqrt( 1 / (2 * pow(10, (SNR_better/10) ) ) );

    //     do {
    //         source_generate(U_K, info_bits);
    //         encoder_repetition_encode(U_K,C_N,info_bits,n_reps);
    //         module_bpsk_modulate(C_N, X_N, codeword_size);
    //         // ADD AWGN HERE
    //         modem_BPSK_demodulate(Y_N, L_N, codeword_size, sigma);
    //         decoder_fn ( L_N, V_K, info_bits, n_reps);
    //         monitor_check_errors(U_K, V_K, info_bits, &n_bit_errors, &n_frame_errors);
    //         n_frame_simulated++;
    //     } while (n_frame_errors < f_max);

    //     end_time = clock();
    //     elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC;
    //     average = elapsed / n_frame_simulated;

    //     fer = n_frame_errors/n_frame_simulated;
    //     ber = n_bit_errors / (n_frame_simulated * info_bits);

    //     // Writing in file
    //     fprintf(file, "%li, %li, %li, %f, %f, %f, %f\n", 
    //         n_bit_errors, n_frame_errors, n_frame_simulated,
    //         ber, fer, elapsed, average);
    // }

    //for (int i=0; i<20; i++) {
        float sigma=0;
        // Generate message
        float Y_N[12] = {-0.2, -1.3, 0.3, 1.2, 0.8, -0.5, -1.5, 0.4, -0.1, 0.2, -1.8, 2};
        float L_N[12] = {0};
        printf("\nTableau genere : ");
        print_array_float(Y_N, 12);

        // Demodulate
        modem_BPSK_demodulate(Y_N,L_N,12,1);
        printf("\nTableau demodule : \n");
        print_array_float(L_N,12);

        // Decode
        uint8_t res[4];
        codec_repetition_hard_decode(L_N, res, 4, 3);
        printf("\nTableau decode : ");
        print_array(res, 4);
        printf("\n__________\n");



    //}

    return 0;
}
