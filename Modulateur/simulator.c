#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <arm_neon.h>
#include <pthread.h>

//Unique and synchronous access to variables
pthread_mutex_t fer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ber_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block0_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block4_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block5_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t block6_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "generate.h"
#include "encoder.h"
#include "modulate.h"
#include "decode.h"
#include "monitor.h"

// Global variables are necessary since we use threads - no parameters routine
//  simulation parameters & default values
float min_SNR = 0;
float max_SNR = 12;
float step_val = 1;
uint32_t f_max = 100;
uint32_t info_bits = 32;
uint32_t codeword_size = 128;
int K, N;

// Parameters for fixed-point functions
char use_fixed = 0;
int f = 3;
int s = 7;

// Parameters for bit packing
char use_packing = 0;

// Computation values
uint64_t n_bit_errors, n_frame_errors, n_frame_simulated; // Frame and bit stats
double sigma;                                             // Variance
float SNR_better;                                         // Es/N0 instead of Eb/N0
float R;                                                  // Ratio - need to cast to float else rounds to ints
uint32_t n_reps;                                          // Number of repetitions
float ber, fer;                                           // Stats - computed after one loop

// Time computation
struct timespec start_time, end_time; // total SNR sim time
int64_t elapsed = 0;            // total time for 1 SNR sim (including all calculations)
float average = 0;            // average time per frame for 1 SNR sim
float sim_thr;                // throughput for 1 SNR sim (Mbps)

// For random noise
const gsl_rng_type *rangentype;
gsl_rng *rangen;
// Per-block statistics
#ifdef ENABLE_STATS
float min_time[7] = {-1};
float max_time[7] = {-1};
float avg_time[7] = {0};
float avg_thr[7] = {0};
float total_time_func = 0; // total time for 1 SNR sim NOT including calculations
#endif

// Function pointers to easily change the function used
void (*decoder_fn_float)(const float *, uint8_t *, size_t, size_t) = codec_repetition_soft_decode;
void (*decoder_fn_fixed)(const int8_t *, uint8_t *, size_t, size_t) = codec_repetition_soft_decode8;
void (*generate_fn)(uint8_t *, size_t) = source_generate;
void (*modulate_fn)(const uint8_t *, int32_t *, size_t) = module_bpsk_modulate;
void (*demodulate_fn)(const float *, float *, size_t, float) = modem_BPSK_demodulate;
void (*monitor_fn)(const uint8_t *, const uint8_t *, size_t, uint64_t *, uint64_t *) = monitor_check_errors;
void (*encoder_fn)(const uint8_t*, uint8_t*, size_t, size_t) = encoder_repetition_encode;

// Filepaths where we store our stats
char filepath[20] = {0};
char filepath_stats[30] = {0};

// Separate function used for threads
void *routine(void *param)
{
    // Arrays & simulation parameters
    // K, N used for variable length depending on bit packing or not
    uint8_t U_K[K];             // Source message
    uint8_t C_N[N];             // Repetition coded message
    int32_t X_N[codeword_size]; // Modulated message
    float Y_N[codeword_size];   // Received message after channel
    float L_N[codeword_size];   // Demodulated message
    int8_t L8_N[codeword_size]; // Demodulated message - fixed point 
    uint8_t V_K[info_bits];     // Decoded message
    uint8_t P_K[K];             // Decoded message repacked

#ifdef ENABLE_STATS

    float cycles = 0;
    clock_t begin_step, end_step; // block time
#endif
// simulate this snr until we reach desired number of errors
    do
    {
// SOURCE GEN - create frame
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        generate_fn(U_K, K);
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block0_mutex);
        avg_time[0] += cycles;
        min_time[0] = ((cycles < min_time[0]) ? cycles : min_time[0]);
        max_time[0] = ((cycles > max_time[0]) ? cycles : max_time[0]);
        pthread_mutex_unlock(&block0_mutex);

        total_time_func += cycles;
#endif

// ENCODE - form codeword (repetitions)
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        encoder_fn(U_K, C_N, K, n_reps);
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block1_mutex);
        avg_time[1] += cycles;
        min_time[1] = ((cycles < min_time[1]) ? cycles : min_time[1]);
        max_time[1] = ((cycles > max_time[1]) ? cycles : max_time[1]);
        pthread_mutex_unlock(&block1_mutex);

        total_time_func += cycles;
#endif

// MODULATE - convert to symbol
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        modulate_fn(C_N, X_N, codeword_size);
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block2_mutex);
        avg_time[2] += cycles;
        min_time[2] = ((cycles < min_time[2]) ? cycles : min_time[2]);
        max_time[2] = ((cycles > max_time[2]) ? cycles : max_time[2]);
        pthread_mutex_unlock(&block2_mutex);
        total_time_func += cycles;
#endif

// CHANNEL - add noise
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        channel_AGWN_add_noise(X_N, Y_N, codeword_size, sigma, rangen);
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block3_mutex);
        avg_time[3] += cycles;
        min_time[3] = ((cycles < min_time[3]) ? cycles : min_time[0]);
        max_time[3] = ((cycles > max_time[3]) ? cycles : max_time[0]);
        pthread_mutex_unlock(&block3_mutex);
        total_time_func += cycles;
#endif

// DEMODULATE - receive from channel
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        demodulate_fn(Y_N, L_N, codeword_size, sigma);
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block4_mutex);
        avg_time[4] += cycles;
        min_time[4] = ((cycles < min_time[4]) ? cycles : min_time[0]);
        max_time[4] = ((cycles > max_time[4]) ? cycles : max_time[0]);
        pthread_mutex_unlock(&block4_mutex);
        total_time_func += cycles;
#endif

        // DECODE - recover message
        if (use_fixed)
        {
            quantizer_transform8(L_N, L8_N, codeword_size, s, f); // Quantizer
#ifdef ENABLE_STATS
            begin_step = clock(); // We don't want to take quantizer time into account
#endif
            decoder_fn_fixed(L8_N, V_K, info_bits, n_reps);
        }
        else
        {
#ifdef ENABLE_STATS
            begin_step = clock();
#endif
            decoder_fn_float(L_N, V_K, info_bits, n_reps);
        }
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block5_mutex);
        avg_time[5] += cycles;
        min_time[5] = ((cycles < min_time[5]) ? cycles : min_time[0]);
        max_time[5] = ((cycles > max_time[5]) ? cycles : max_time[0]);
        pthread_mutex_unlock(&block5_mutex);
        total_time_func += cycles;
#endif
        if (use_packing) {
            bit_packer(V_K, P_K, K);
        }

// MONITOR - error check
#ifdef ENABLE_STATS
        begin_step = clock();
#endif
        if (use_packing) {
            monitor_fn(U_K, P_K, info_bits/8, &n_bit_errors, &n_frame_errors);
        } else {
            monitor_fn(U_K, V_K, info_bits, &n_bit_errors, &n_frame_errors);
        }
#ifdef ENABLE_STATS
        end_step = clock();
        cycles = ((end_step - begin_step) * 1000000) / CLOCKS_PER_SEC;
        pthread_mutex_lock(&block6_mutex);
        avg_time[6] += cycles;
        min_time[6] = ((cycles < min_time[6]) ? cycles : min_time[6]);
        max_time[6] = ((cycles > max_time[6]) ? cycles : max_time[6]);
        pthread_mutex_unlock(&block6_mutex);
        total_time_func += cycles;
#endif
        pthread_mutex_lock(&frame_mutex);
        n_frame_simulated++;
        pthread_mutex_unlock(&frame_mutex);
    } while (n_frame_errors < f_max);
    return NULL;
}


//Measures the elapsed time im us
//Code by Glärbo on StackOverflow (https://stackoverflow.com/questions/64893834/measuring-elapsed-time-using-clock-gettimeclock-monotonic)
int64_t difftimespec_us(const struct timespec after, const struct timespec before)
{
    return ((int64_t)after.tv_sec - (int64_t)before.tv_sec) * (int64_t)1000000
         + ((int64_t)after.tv_nsec - (int64_t)before.tv_nsec) / 1000;
}

int main(int argc, char **argv)
{
    char threads = 0;
    // For long option - we set an alias
    struct option zero_opt[5] = {{"src-all-zeros", no_argument, NULL, 'z'},
                                 {"demod-neon", no_argument, NULL, 'a'},
                                 {"qf", required_argument, NULL, 'g'},
                                 {"qs", required_argument, NULL, 'h'},
                                 {0, 0, 0, 0}};
    int long_index = 0;
    // We use getopt to parse command line arguments
    // An option is a parameter beginning with '-' (different from only "-" or starting with "--")
    // getopt (argc, argv, format)
    // Format is a string containing all valid options (as letters) to be parsed
    // If one of those chars is followed by a ':', it means it expects an argument after
    // It is the case for all of our arguments
    // The following argument is then stored in optarg variable
    int opt;
    // Loops while something to read
    while ((opt = getopt_long(argc, argv, "m:M:s:e:K:N:D:f:o:c:tp", zero_opt, &long_index)) != -1)
    {
        switch (opt)
        {
        // Minimum SNR
        case 'm':
            min_SNR = atof(optarg);
            break;
        // Maximum SNR
        case 'M':
            if ((max_SNR = atof(optarg)) == 0)
                max_SNR = 12;
            break;
        // Step between each SNR
        case 's':
            if ((step_val = atof(optarg)) == 0)
                step_val = 1;
            break;
        // Number of frame error to get
        case 'e':
            if ((f_max = atoi(optarg)) == 0)
                f_max = 100;
            break;
        // Number of info bits
        case 'K':
            if ((info_bits = atoi(optarg)) == 0)
                info_bits = 32;
            break;
        // Number of bits in message sent
        case 'N':
            if ((codeword_size = atoi(optarg)) == 0)
                codeword_size = 128;
            break;
        // Monitor version to use
        case 'c':
            if (strcmp(optarg, "monitor-neon") == 0) {
                printf("use of monitor neon\n");
                monitor_fn = monitor_neon;
            }
        // Decoder version to use
        case 'D':
            if (strcmp(optarg, "rep-hard") == 0)
            {
                use_fixed = 0;
                decoder_fn_float = codec_repetition_hard_decode;
            }
            else if (strcmp(optarg, "rep-hard8") == 0)
            {
                use_fixed = 1;
                decoder_fn_fixed = codec_repetition_hard_decode8;
            }
            else if (strcmp(optarg, "rep-soft8") == 0)
            {
                use_fixed = 1;
                decoder_fn_fixed = codec_repetition_soft_decode8;
            }
            else if (strcmp(optarg, "rep-hard8-neon") == 0)
            {
                use_fixed = 1;
                decoder_fn_fixed = codec_repetition_hard_decode8_neon;
            }
            else if (strcmp(optarg, "rep-soft8-neon") == 0)
            {
                use_fixed = 1;
                decoder_fn_fixed = codec_repetition_soft_decode8_neon;
            }
            break;
        // Name of the file to store results in
        case 'f':
            sprintf(filepath, "sim_%s.csv", optarg);
            sprintf(filepath_stats, "sim_%s_stats.csv", optarg);
            break;
        // Modulator function to use
        case 'o':
            if (strcmp(optarg, "mod-all-ones") == 0)
            {
                modulate_fn = modem_BPSK_modulate_all_ones;
            }
            else if (strcmp(optarg, "mod-neon") == 0)
            {  
                printf("use of mod neon\n");
                modulate_fn = module_bpsk_modulate_neon;
            }
            break;
        // Generator function to use - see long arg
        case 'z':
            generate_fn = source_generate_all_zeros;
            break;
        // Demodulator function to use - see long arg
        case 'a': // Check long
            printf("Use of demod neon\n");
            demodulate_fn = modem_BPSK_demodulate_neon;
            break;
        // Number of "decimals" in quantisized value
        case 'g': // float
            use_fixed = 1;
            f = atoi(optarg);
            break;
        // Total number of bits in quantisized value
        case 'h':
            // Using qs should do nothing if we didn't call qf -- no use_float = 1 here
            s = atoi(optarg);
            break;
        // threading
        case 't':
            printf("use of threads\n");
            threads = 1;
            break;
        // bit packing
        case 'p':
            printf("bit packing enabled - generate and modulate function selection ignored\n");
            use_packing = 1;
        }
    }

    // if using bit packing, have to use special modulator (can use any demod tho)
    if (use_packing) {
        generate_fn = source_gen_bit_pack;
        encoder_fn = encoder_rep_encode_bit_pack;
        modulate_fn = module_bpsk_modulate_bit_unpack;
    }

    if (use_packing) {
        K = info_bits/8;
        N = codeword_size/8;
    } else {
        K = info_bits;
        N = codeword_size;
    }

    // check s and f values
    char e = 0;
    if (use_fixed)
    {
        if (s > 7)
        {
            printf("ERROR: maximum s value is 7\n");
            e = 1;
        }
        if (s <= f + 1)
        {
            printf("ERROR: s must be greater than f+1\n");
            e = 1;
        }
    }
    if (e)
        return 1;

    // Init output files
    FILE *file;
    if (filepath[0] == 0)
        file = stdout;
    else
        file = fopen(filepath, "w");
    fprintf(file, "Eb/No,Es/No,Sigma,# Bit Errors,# Frame Errors,# Simulated frames,BER,FER,Time for this SNR,Average time for one frame,SNR throughput\n");

#ifdef ENABLE_STATS
    FILE *file_stats;
    if (filepath_stats[0] == 0)
        file_stats = stdout;
    else
        file_stats = fopen(filepath_stats, "w");
    fprintf(file_stats, "Eb/No,gen_avg,gen_min,gen_max,gen_thr,gen_percent,encode_avg,encode_min,encode_max,encode_thr,encode_percent,bpsk_avg,bpsk_min,bpsk_max,bpsk_thr,bpsk_percent,awgn_avg,awgn_min,awgn_max,awgn_thr,awgn_percent,demodulate_avg,demodulate_min,demodulate_max,demodulate_thr,demodulate_percent,decode_avg,decode_min,decode_max,decode_thr,decode_percent,monitor_avg,monitor_min,monitor_max,monitor_thr,monitor_percent\n");

    // blocks used :          source gen, encode,        modulate,      channel,       demodulate,    decode,    monitor
    uint32_t block_bits[7] = {info_bits, codeword_size, codeword_size, codeword_size, codeword_size, info_bits, info_bits}; // number of bits for each block
#endif

    // Init constants given our inputs
    R = (float)info_bits / codeword_size; // Ratio - need to cast to float else rounds to ints
    n_reps = codeword_size / info_bits;   // Number of repetitions

    // Init random - for generation and normal law
    srand(time(NULL)); // Initialization, should only be called once.
    rangentype = gsl_rng_default;
    rangen = gsl_rng_alloc(rangentype); // random number gen w uniform distr

    // Start loop
    for (float val = min_SNR; val <= max_SNR; val += step_val)
    {
        // Init our Es/No and sigma
        SNR_better = val + 10 * log10f(R); // have to use log10 not just log
        sigma = sqrt(1 / (2 * pow(10, (SNR_better / 10))));

        // Reset stats
        n_bit_errors = 0;
        n_frame_errors = 0;
        n_frame_simulated = 0;

#ifdef ENABLE_STATS // Reset stats only if stats enabled
        for (int i = 0; i < 7; i++)
        {
            min_time[i] = INFINITY;
            max_time[i] = -INFINITY;
            avg_time[i] = 0;
        }

        total_time_func = 0;
#endif

        //printf("current snr = %f, min snr = %f, max snr = %f, sigma = %f\n", val, min_SNR, max_SNR, sigma);
        clock_gettime(CLOCK_REALTIME, &start_time);
        pthread_t t0, t1, t2, t3, t4;
        if (threads)
        {
            pthread_create(&t0, NULL, routine, NULL);
            pthread_create(&t1, NULL, routine, NULL);
            pthread_create(&t2, NULL, routine, NULL);
            pthread_create(&t3, NULL, routine, NULL);
            pthread_create(&t4, NULL, routine, NULL);
        }
        routine(NULL);
        if (threads)
        {
            pthread_join(t0, NULL);
            pthread_join(t1, NULL);
            pthread_join(t2, NULL);
            pthread_join(t3, NULL);
            pthread_join(t4, NULL);
        }

        // End stats
        clock_gettime(CLOCK_REALTIME, &end_time);
        elapsed = difftimespec_us(end_time, start_time);
        average = (float)elapsed / (float)n_frame_simulated;

        fer = (float)n_frame_errors / n_frame_simulated;
        ber = (float)n_bit_errors / (n_frame_simulated * info_bits);
        sim_thr = (float)n_frame_simulated * info_bits / elapsed; // throughput in Mbps

// Block stats display
#ifdef ENABLE_STATS
        for (int i = 0; i < 7; i++)
        {
            avg_time[i] = (float)avg_time[i] / n_frame_simulated;
        }
        for (int i = 0; i < 7; i++)
        {
            avg_thr[i] = (float)block_bits[i] / avg_time[i];
        }

        fprintf(file_stats, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", val,
                avg_time[0], min_time[0], max_time[0], avg_thr[0], avg_time[0] * 100 / (average / n_frame_simulated),
                avg_time[1], min_time[1], max_time[1], avg_thr[1], avg_time[1] * 100 / (average / n_frame_simulated),
                avg_time[2], min_time[2], max_time[2], avg_thr[2], avg_time[2] * 100 / (average / n_frame_simulated),
                avg_time[3], min_time[3], max_time[3], avg_thr[3], avg_time[3] * 100 / (average / n_frame_simulated),
                avg_time[4], min_time[4], max_time[4], avg_thr[4], avg_time[4] * 100 / (average / n_frame_simulated),
                avg_time[5], min_time[5], max_time[5], avg_thr[5], avg_time[5] * 100 / (average / n_frame_simulated),
                avg_time[6], min_time[6], max_time[6], avg_thr[6], avg_time[6] * 100 / (average / n_frame_simulated));
        fflush(file_stats);
#endif

        // Writing in file
        fprintf(file, "%f, %f, %f, %lu, %lu, %lu, %f, %f, %ld, %f, %f\n",
                val, SNR_better, sigma,
                n_bit_errors, n_frame_errors, n_frame_simulated,
                ber, fer, elapsed, average, sim_thr);
        fflush(file);
    }
    printf("ended\n");
    gsl_rng_free(rangen);
    pthread_mutex_destroy(&fer_mutex);
    pthread_mutex_destroy(&ber_mutex);
    return 0;
}
