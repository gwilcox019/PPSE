#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

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

void print_array_float (float* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%f ; ", array[i]);
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
void module_bpsk_modulate (const uint8_t* CN, int32_t* XN, size_t n) {
    for (; n>0; n--)
        XN[n-1] = (CN[n-1]?-1:1);
}

// adds random noise following a normal distribution
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma) {
    const gsl_rng_type * rangentype;
    rangentype = gsl_rng_default;
    gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 
    for (; n>0; n--) {
        float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
        Y_N[n-1] = X_N[n-1] + v;
    }
    gsl_rng_free (rangen);
}

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma){
        memcpy (L_N, Y_N, n*sizeof(float));
}

void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t k, size_t n_reps) {
    // Reduce float to corresponding int (-1 ; 1)
    // Then average out the hard decisions by summing them
    int8_t average[K] = {0}; 
    for (int i=0; i<k; k++) {
	    for (int j= 0; j<n_reps; j++) {
            average[i] += (L_N[i+j*k] >= 0 ? 1 : -1) ;
        }
        // Map hard decision to decoded message
        V_N[i] = (average[i]>=0?0:1);

    }
}

void codec_repetition_soft_decode (const float* L_N, uint8_t *V_N, size_t k, size_t n_reps) {

}

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors) {
    int flag = 0;
    for (; k>0; k--) {
        if (U_K[k] != V_K[k]) {
            if (!flag) { *n_frame_errors++; flag++; }
            n_bit_errors++;
        }
    }
}



int main( int argc, char** argv) {
    // simulation parameters
    float min_SNR = 0;
    float max_SNR = 12;
    float step_val = 1;
    uint32_t f_max = 100;
    uint32_t info_bits = 32;
    uint32_t codeword_size = 128;
    void (*decoder_fn) (const float*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode; 
    char filepath[6] = {0};

    // We use getopt to parse command line arguments
    // An option is a parameter beginning with '-' (different from "-" and "--")
    // getopt (argc, argv, format)
    // Format is a string containing all valid options (as letters) to be parsed
    // If one of those chars is followed by a ':', it means it expects an argument after
    // It is the case for all of our arguments
    // The following argument is then stored in optarg variable
    int opt;
    //Loops while something to read
    while ((opt = getopt(argc, argv, "m:M:s:e:K:N:D:s:")) != -1) {
        switch(opt) {
            case 'm': min_SNR = atof(optarg); break;
            case 'M': if ((max_SNR = atof(optarg)) == 0) max_SNR = 12; break;
            case 's': if ((step_val = atof(optarg)) == 0) step_val = 1; break;
            case 'e': if ((f_max = atoi(optarg)) == 0) f_max = 100; break;
            case 'K': if ((info_bits = atoi(optarg)) == 0) info_bits = 32; break;
            case 'N': if ((codeword_size = atoi(optarg)) == 0) codeword_size = 128; break;
            case 'D':
                if (strcmp(optarg, "rep-hard") == 0) decoder_fn = codec_repetition_hard_decode;
                break;
            case 's': sprintf(filepath, "sim_%i.csv", atoi(optarg));
                
        }
    }

    // Arrays & simulation parameters
    uint8_t UK[info_bits]; // Source message
    uint8_t CN[codeword_size]; // Repetition coded message
    int32_t XN[codeword_size]; // Modulated message
    float Y_N[codeword_size];  // Received message after canal
    float L_N[codeword_size];  // Demodulated message
    uint8_t V_N[info_bits];// Decoded message

    uint64_t n_bit_errors, n_frame_errors, n_frame_simulated; // Frame and bit stats
    float sigma; // Variance
    float SNR_better; // Es/N0 instead of Eb/N0
    float R = K/N; // Ratio
    uint32_t n_reps = N/K; //Number of repetitions

    // Stats - computed after one loop
    float ber, fer;

    // Output file
    int fd;
    if (filepath[0] == 0) fd = stdout;
    else fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC);
    dprintf(fd, "# Bit Errors, # Frame Errors, # Simulated frames, BER, FER, Time for this SNR, Average time for one frame\n");

    // Time computation
    clock_t start_time, end_time;
    float elapsed=0;
    float average=0;
    
    //Init random
    srand(time(NULL));   // Initialization, should only be called once.

    for (float val = min_SNR; val <= max_SNR; val+=step_val) {
        start_time = clock();
        n_bit_errors = 0;
        n_frame_errors = 0;
        n_frame_simulated = 0;

        SNR_better = val + 10*log(R*1);
        sigma = sqrt( 1 / (2 * pow(10, (SNR_better/10) ) ) )

        do {
            start_frame = clock()
            source_generate(UK, K);
            encoder_repetition_encode(UK,CN,K,REPS);
            module_bpsk_modulate(CN, XN, N);
            // ADD AWGN HERE
            modem_BPSK_demodulate(Y_N, L_N, N, sigma);
            decoder_fn ( L_N, V_N, K, n_reps);
            monitor_check_errors(U_K, V_K, K, &n_bit_errors, &n_frame_errors);
            n_frame_simulated++;
        } while (n_frame_errors < f_max);

        end_time = clock();
        elapsed = (float) (end_time - start_time) / CLOCKS_PER_SEC;
        average = elapsed / n_frame_simulated;

        fer = n_frame_errors/n_frame_simulated;
        ber = n_bit_errors / (n_frame_simulated * K);

        // Writing in file
        dprintf(fd, "%i, %i, %i, %f, %f, %f, %f\n", 
            n_bit_errors, n_frame_errors, n_frames_simulated,
            ber, fer, elapsed, average);
    }

    // test
    // for (int i=0; i<20; i++) {
    //     float sigma=0;
    //     // Generate message
    //     source_generate(UK, K);
    //     printf("\nTableau genere : ");
    //     print_array(UK, K);

    //     // Encoded message
    //     encoder_repetition_encode(UK,CN,K,REPS);
    //     printf("\nTableau encode : ");
    //     print_array(CN,N);

    //     // Modulated message
    //     module_bpsk_modulate(CN, XN, N);
    //     printf("\nTableau module : ");
    //     print_array_32(XN, N);
    //     printf("\n__________\n");

    //     // Canal message
    //     // TODO
    //     printf("\nTableau transmis : ");
    //     print_array_float(Y_N, N);

    //     // Demodulated message
    //     modem_BPSK_demodulate(Y_N, L_N, N, sigma);



    // }

    return 0;
}
