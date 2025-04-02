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
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <getopt.h>


// gcc simulator.c -o simulator.x -Wall -std=c99 -I/usr/include/gsl -lgsl -lgslcblas -lm
// (change I flag for where gsl is on the machine)

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

// alternative generator with all zero
void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    memset(U_K, 0, K*sizeof(uint8_t));
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

void modem_BPSK_modulate_all_ones(const uint8_t *C_N, int32_t *X_N, size_t N) {
    memset(X_N, 1, N*sizeof(int32_t));
}

// adds random noise following a normal distribution
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen) {
    for (; n>0; n--) {
        float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
        Y_N[n-1] = X_N[n-1] + v;
    }

}

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma){
        memcpy (L_N, Y_N, n*sizeof(float));
}

// A FAIRE PAR GRACE
void quantizer_transform8 (cost float* L_N, int8_t* L8_N, size_t N, size_t s, size_t f) {

}

void codec_repetition_hard_decode (const float* L_N, uint8_t* V_N, size_t k, size_t n_reps) {
    // Reduce float to corresponding int (-1 ; 1)
    // Then average out the hard decisions by summing them
    int average = 0;
    for (int i=0; i<k; i++) {
        average = 0;
	    for (int j= 0; j<n_reps; j++) {
            average += (L_N[i+j*k] >= 0 ? 1 : -1) ;
        }
        // Map hard decision to decoded message
        V_N[i] = (average>=0?0:1);
    }
}

// 12 elems dans tableau, k = 4, reps = 3
// Elements à additionner : 0-4-8-12 / 1-5-9-13 / 2-6-10-14 / 3-7-11-15
// on parcourt k
//

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

void monitor_check_errors (const uint8_t* U_K, const uint8_t *V_K, size_t k, uint64_t *n_bit_errors, uint64_t *n_frame_errors) {
    int flag = 0;
    for (int i=0; i<k; i++) {
        if (U_K[i] != V_K[i]) {
            if (!flag) { 
                *n_frame_errors = *n_frame_errors+1; 
                flag++; 
            }
            *n_bit_errors = *n_bit_errors+1;
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
    void  (*decoder_fn) (const float*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode;
    void (*generate_fn) (uint8_t*, size_t) = source_generate;
    void (*modulate_fn) (const uint8_t*, int32_t*, size_t) = module_bpsk_modulate;
    char filepath[11] = {0};
    char filepath_stats[17] = {0};
    // For long option - we set an alias
    struct option zero_opt[3] = {{"src-all-zeros", no_argument, NULL, 'z'}, {"mod-all-ones", no_argument, NULL, 'o'},{0,0,0,0}};
    int long_index=0;
    // We use getopt to parse command line arguments
    // An option is a parameter beginning with '-' (different from only "-" or starting with "--") 
    // getopt (argc, argv, format)
    // Format is a string containing all valid options (as letters) to be parsed
    // If one of those chars is followed by a ':', it means it expects an argument after
    // It is the case for all of our arguments
    // The following argument is then stored in optarg variable
    int opt;
    //Loops while something to read
    while ((opt = getopt_long(argc, argv, "m:M:s:e:K:N:D:f:", zero_opt, &long_index)) != -1) {
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
            case 'f': sprintf(filepath, "sim_%i.csv", atoi(optarg)); sprintf(filepath_stats, "sim_%i_stats.csv", atoi(optarg)); break;
            case 'z': //Check long
                generate_fn = source_generate_all_zeros;
		printf("whanged source\n");
                break;
            case 'o': //check long
                modulate_fn = modem_BPSK_modulate_all_ones; 
		printf("changed modulate\n");
		break;
        }
    }

    // Arrays & simulation parameters
    uint8_t U_K[info_bits]; // Source message
    uint8_t C_N[codeword_size]; // Repetition coded message
    int32_t X_N[codeword_size]; // Modulated message
    float Y_N[codeword_size];  // Received message after canal
    float L_N[codeword_size];  // Demodulated message
    uint8_t V_K[info_bits];// Decoded message

    uint64_t n_bit_errors, n_frame_errors, n_frame_simulated; // Frame and bit stats
    double sigma; // Variance
    float SNR_better; // Es/N0 instead of Eb/N0
    float R = (float)info_bits/codeword_size; // Ratio - need to cast to float else rounds to ints
    uint32_t n_reps = codeword_size/info_bits; //Number of repetitions

    // blocks used :          source gen, encode,        modulate,      channel,       demodulate,    decode,    monitor
    uint32_t block_bits[7] = {info_bits,  codeword_size, codeword_size, codeword_size, codeword_size, info_bits, info_bits}; // number of bits for each block

    // Stats - computed after one loop
    float ber, fer;

    // Output file
    FILE* file;
    if (filepath[0] == 0) file = stdout;
    else file = fopen(filepath, "w");
    FILE* file_stats;
    if (filepath[0] == 0) file_stats = stdout;
    else file_stats = fopen(filepath_stats, "w");
    fprintf(file, "Eb/No,Es/No,Sigma,# Bit Errors,# Frame Errors,# Simulated frames,BER,FER,Time for this SNR,Average time for one frame, SNR throughput\n");
    fprintf(file_stats, "gen_avg,gen_min,gen_max,gen_thr,gen_percent,encode_avg,encode_min,encode_max,encode_thr,encode_percent,bpsk_avg,bpsk_min,bpsk_max,bpsk_thr,bpsk_percent,awgn_avg,awgn_min,awgn_max,awgn_thr,awgn_percent,demodulate_avg,demodulate_min_demodulate_max,demodulate_thr,demodulate_percent,decode_avg,decode_min,decode_max,decode_thr,decode_percent,monitor_avg,monitor_min,monitor_max,monitor_thr,monitor_percent\n");
    // Time computation
    clock_t start_time, end_time; // total SNR sim time
    clock_t begin_step, end_step; // block times
    float elapsed=0; // total time for 1 SNR sim (including all calculations)
    float total_time_func = 0; // total time for 1 SNR sim NOT including calculations
    float average=0; // average time per frame for 1 SNR sim
    float sim_thr; // throughput for 1 SNR sim (Mbps)

    //Per-block statistics
    #ifdef ENABLE_STATS
    float min_time[7] = {-1};
    float max_time[7] = {-1};
    float avg_time[7] = {0};
    float avg_thr[7] = {0};
    int cycles = 0;
    #endif
    
    //Init random
    srand(time(NULL));   // Initialization, should only be called once.
    const gsl_rng_type * rangentype;
    rangentype = gsl_rng_default;
    gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 

    for (float val = min_SNR; val <= max_SNR; val+=step_val) {
        start_time = clock();
        n_bit_errors = 0;
        n_frame_errors = 0;
        n_frame_simulated = 0;
        total_time_func = 0;
        
        #ifdef ENABLE_STATS
        memset(min_time, -1, 7*sizeof(float));
        memset(max_time, -1, 7*sizeof(float));
        memset(avg_time, 0, 7*sizeof(float));
        #endif

        SNR_better = val + 10*log10f(R); // have to use log10 not just log
        sigma = sqrt( 1 / (2 * pow(10, (SNR_better/10) ) ) ); 

        printf("current snr = %f, min snr = %f, max snr = %f, sigma = %f\n", min_SNR, max_SNR, val, sigma);

        // simulate this snr until we reach desired number of errors
        do {
            // SOURCE GEN - create frame
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            generate_fn(U_K, info_bits);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[0] += (end_step-begin_step);
	    
            min_time[0]  = ((min_time[0] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[0]  = ((max_time[0] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif

            // ENCODE - form codeword (repetitions)
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            encoder_repetition_encode(U_K,C_N,info_bits,n_reps);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[1] += (end_step-begin_step);
            min_time[1]  = ((min_time[1] == -1 || (end_step-begin_step) < min_time[1]) ? (end_step-begin_step) : min_time[1]);
            max_time[1]  = ((max_time[1] == -1 || (end_step-begin_step) > max_time[1]) ? (end_step-begin_step) : max_time[1]);
            total_time_func += (end_step-begin_step);
            #endif

            // MODULATE - convert to symbol
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            modulate_fn(C_N, X_N, codeword_size);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[2] += (end_step-begin_step);
            min_time[2]  = ((min_time[2] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[2]  = ((max_time[2] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif
            
            // CHANNEL - add noise
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            channel_AGWN_add_noise(X_N, Y_N, codeword_size, sigma, rangen);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[3] += (end_step-begin_step);
            min_time[3]  = ((min_time[3] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[3]  = ((max_time[3] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif

            // DEMODULATE - receive from channel
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            modem_BPSK_demodulate(Y_N, L_N, codeword_size, sigma);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[4] += (end_step-begin_step);
            min_time[4]  = ((min_time[4] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[4]  = ((max_time[4] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif

            // DECODE - recover message 
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            decoder_fn ( L_N, V_K, info_bits, n_reps);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[5] += (end_step-begin_step);
            min_time[5]  = ((min_time[5] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[5]  = ((max_time[5] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif

            // MONITOR - error check
            #ifdef ENABLE_STATS
             begin_step = micros();
            #endif
            monitor_check_errors(U_K, V_K, info_bits, &n_bit_errors, &n_frame_errors);
            #ifdef ENABLE_STATS
            end_step = micros(); 
            avg_time[6] += (end_step-begin_step);
            min_time[6]  = ((min_time[6] == -1 || (end_step-begin_step) < min_time[0]) ? (end_step-begin_step) : min_time[0]);
            max_time[6]  = ((max_time[6] == -1 || (end_step-begin_step) > max_time[0]) ? (end_step-begin_step) : max_time[0]);
            total_time_func += (end_step-begin_step);
            #endif

            n_frame_simulated++;
        } while (n_frame_errors < f_max);

        end_time = clock();
        elapsed = (float) (end_time - start_time) / (CLOCKS_PER_SEC*1000000); // microseconds
        average = elapsed / n_frame_simulated;

        fer = (float)n_frame_errors/n_frame_simulated;
        ber = (float)n_bit_errors / (n_frame_simulated * info_bits);
        sim_thr = (float)n_frame_simulated * info_bits / elapsed; // throughput in Mbps

        // Block stats display
        #ifdef ENABLE_STATS
        for (int i=0; i<7; i++) {
            avg_time[i] = (float) avg_time[i]/n_frame_simulated;
        }
        for (int i=0; i<7; i++) {
            avg_thr[i] = (float) block_bits[i]/avg_time[i];
        }

        fprintf(file_stats,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            avg_time[0], min_time[0], max_time[0], avg_thr[0], avg_time[0] * 100/(total_time_func/n_frame_simulated),
            avg_time[1], min_time[1], max_time[1], avg_thr[1], avg_time[1] * 100/(total_time_func/n_frame_simulated),
            avg_time[2], min_time[2], max_time[2], avg_thr[2], avg_time[2] * 100/(total_time_func/n_frame_simulated),
            avg_time[3], min_time[3], max_time[3], avg_thr[3], avg_time[3] * 100/(total_time_func/n_frame_simulated),
            avg_time[4], min_time[4], max_time[4], avg_thr[4], avg_time[4] * 100/(total_time_func/n_frame_simulated),
            avg_time[5], min_time[5], max_time[5], avg_thr[5], avg_time[5] * 100/(total_time_func/n_frame_simulated),
            avg_time[6], min_time[6], max_time[6], avg_thr[6], avg_time[6] * 100/(total_time_func/n_frame_simulated) );
	fflush(file_stats);
        #endif

        // Writing in file
        fprintf(file, "%f, %f, %f, %li, %li, %li, %f, %f, %f, %f, %f\n", 
            val, SNR_better, sigma,
            n_bit_errors, n_frame_errors, n_frame_simulated,
            ber, fer, elapsed, average, sim_thr);
	fflush(file);
    }
    gsl_rng_free (rangen);
    return 0;
}
