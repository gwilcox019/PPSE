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

//#include "debug_func.h"
#include "generate.h"
#include "encoder.h"
#include "modulate.h"
#include "decode.h"
#include "monitor.h"

int main( int argc, char** argv) {
    
    // simulation parameters & default values
    float min_SNR = 0;
    float max_SNR = 12;
    float step_val = 1;
    uint32_t f_max = 100;
    uint32_t info_bits = 32;
    uint32_t codeword_size = 128;

    // Parameters for fixed-point functions
    char use_fixed = 0;
    int f = 3;
    int s = 7;

    //Function pointers to easily change the function used
    void (*decoder_fn_float) (const float*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode;
    void (*decoder_fn_fixed) (const int8_t*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode8;
    void (*generate_fn) (uint8_t*, size_t) = source_generate;
    void (*modulate_fn) (const uint8_t*, int32_t*, size_t) = module_bpsk_modulate;
    void (*demodulate_fn) (const float*, float*, size_t, float) = modem_BPSK_demodulate;
    void (*monitor_fn) (const uint8_t*, const uint8_t *, size_t, uint64_t *, uint64_t *) = monitor_check_errors;

    //Filepaths where we store our stats
    char filepath[20] = {0};
    char filepath_stats[30] = {0};


    // For long option - we set an alias
    struct option zero_opt[5] = {{"src-all-zeros", no_argument, NULL, 'z'}, 
                                {"demod-neon", no_argument, NULL, 'a'},
                                {"qf", required_argument, NULL, 'g'},
                                {"qs", required_argument, NULL, 'h'},
                                {0,0,0,0}};
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
    while ((opt = getopt_long(argc, argv, "m:M:s:e:K:N:D:f:o:c:", zero_opt, &long_index)) != -1) {
        switch(opt) {
            case 'm': min_SNR = atof(optarg); break;
            case 'M': if ((max_SNR = atof(optarg)) == 0) max_SNR = 12; break;
            case 's': if ((step_val = atof(optarg)) == 0) step_val = 1; break;
            case 'e': if ((f_max = atoi(optarg)) == 0) f_max = 100; break;
            case 'K': if ((info_bits = atoi(optarg)) == 0) info_bits = 32; break;
            case 'N': if ((codeword_size = atoi(optarg)) == 0) codeword_size = 128; break;
	    case 'c': if (strcmp(optarg, "monitor-neon") == 0) monitor_fn = monitor_neon;  
            case 'D':
                if (strcmp(optarg, "rep-hard") == 0) {
                    use_fixed = 0;
                    decoder_fn_float = codec_repetition_hard_decode;
                } else if (strcmp(optarg, "rep-hard8") == 0) {
                    use_fixed = 1;
                    decoder_fn_fixed = codec_repetition_hard_decode8;
                } else if (strcmp(optarg, "rep-soft8") == 0) {
                    use_fixed = 1;
                    decoder_fn_fixed = codec_repetition_soft_decode8;
                } else if (strcmp(optarg, "rep-hard8-neon") == 0) {
                    use_fixed = 1;
                    decoder_fn_fixed = codec_repetition_hard_decode8_neon;
                } else if (strcmp(optarg, "rep-soft8-neon") == 0) {
                    use_fixed = 1;
                    decoder_fn_fixed = codec_repetition_soft_decode8_neon;
                } 
                break;
            case 'f': sprintf(filepath, "sim_%s.csv", optarg); sprintf(filepath_stats, "sim_%s_stats.csv",optarg); break;
            case 'o': 
                if (strcmp(optarg, "mod-all-ones") == 0) {
                    modulate_fn = modem_BPSK_modulate_all_ones; 
                } else if (strcmp(optarg, "mod-neon") == 0) {
                    modulate_fn = module_bpsk_modulate_neon;
                }
                break;
            case 'z': //Check long
                generate_fn = source_generate_all_zeros;
                break;
            case 'a': //Check long
                demodulate_fn = modem_BPSK_demodulate_neon;
                break;
            case 'g': //float
                printf("DEBUG : qf called with argument %s\n", optarg);
                use_fixed = 1;
                f = atoi(optarg);
                break;
            case 'h':
                printf("DEBUG : qs called with argument %s\n", optarg);
                // Using qs should do nothing if we didn't call qf -- no use_float = 1 here
                s = atoi(optarg);
                break;
        }
    }

    // check s and f values
    char e = 0;
    if (use_fixed) {
        if (s > 7) {
            printf("ERROR: maximum s value is 7\n");
            e = 1;
        }
        if (s <= f+1) {
            printf("ERROR: s must be greater than f+1\n");
            e = 1;
        }
    }
    if (e == 1) return 1;

    // Arrays & simulation parameters
    uint8_t U_K[info_bits]; // Source message
    uint8_t C_N[codeword_size]; // Repetition coded message
    int32_t X_N[codeword_size]; // Modulated message
    float Y_N[codeword_size];  // Received message after canal
    float L_N[codeword_size];  // Demodulated message
    int8_t L8_N[codeword_size];  // Demodulated message
    uint8_t V_K[info_bits];// Decoded message

    uint64_t n_bit_errors, n_frame_errors, n_frame_simulated; // Frame and bit stats
    double sigma; // Variance
    float SNR_better; // Es/N0 instead of Eb/N0
    float R = (float)info_bits/codeword_size; // Ratio - need to cast to float else rounds to ints
    uint32_t n_reps = codeword_size/info_bits; //Number of repetitions

    // Stats - computed after one loop
    float ber, fer;

    // Output file
    FILE* file;
    if (filepath[0] == 0) file = stdout;
    else file = fopen(filepath, "w");
    fprintf(file, "Eb/No,Es/No,Sigma,# Bit Errors,# Frame Errors,# Simulated frames,BER,FER,Time for this SNR,Average time for one frame,SNR throughput\n");

    FILE* file_stats;
    if (filepath_stats[0] == 0) file_stats = stdout;
    else file_stats = fopen(filepath_stats, "w");
    fprintf(file_stats, "Eb/No,gen_avg,gen_min,gen_max,gen_thr,gen_percent,encode_avg,encode_min,encode_max,encode_thr,encode_percent,bpsk_avg,bpsk_min,bpsk_max,bpsk_thr,bpsk_percent,awgn_avg,awgn_min,awgn_max,awgn_thr,awgn_percent,demodulate_avg,demodulate_min,demodulate_max,demodulate_thr,demodulate_percent,decode_avg,decode_min,decode_max,decode_thr,decode_percent,monitor_avg,monitor_min,monitor_max,monitor_thr,monitor_percent\n");
    
    // Time computation
    clock_t start_time, end_time; // total SNR sim time
    float elapsed=0; // total time for 1 SNR sim (including all calculations)
    float average=0; // average time per frame for 1 SNR sim
    float sim_thr; // throughput for 1 SNR sim (Mbps)

    //Per-block statistics
    #ifdef ENABLE_STATS
    // blocks used :          source gen, encode,        modulate,      channel,       demodulate,    decode,    monitor
    uint32_t block_bits[7] = {info_bits,  codeword_size, codeword_size, codeword_size, codeword_size, info_bits, info_bits}; // number of bits for each block
    float min_time[7] = {-1};
    float max_time[7] = {-1};
    float avg_time[7] = {0};
    float avg_thr[7] = {0};
    float cycles = 0;
    clock_t begin_step, end_step; // block time    
    #endif
    
    float total_time_func = 0; // total time for 1 SNR sim NOT including calculations
			       //
    //Init random - for generation and normal law
    srand(time(NULL));   // Initialization, should only be called once.
    const gsl_rng_type * rangentype;
    rangentype = gsl_rng_default;
    gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 

    //Start loop
    for (float val = min_SNR; val <= max_SNR; val+=step_val) {
        //Reset stats
        start_time = clock();
        n_bit_errors = 0;
        n_frame_errors = 0;
        n_frame_simulated = 0;
        total_time_func = 0;
        
        #ifdef ENABLE_STATS //Reset stats only if stats enabled
        for (int i=0; i<7; i++) {
            min_time[i] = INFINITY;
            max_time[i] = -INFINITY;
            avg_time[i] = 0;
        }
        #endif

        //Init our Es/No and sigma
        SNR_better = val + 10*log10f(R); // have to use log10 not just log
        sigma = sqrt( 1 / (2 * pow(10, (SNR_better/10) ) ) ); 

        printf("current snr = %f, min snr = %f, max snr = %f, sigma = %f\n", val, min_SNR, max_SNR, sigma);

        // simulate this snr until we reach desired number of errors
        do {
            // SOURCE GEN - create frame
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            generate_fn(U_K, info_bits);
	        #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[0] += cycles;
            min_time[0]  = ((cycles < min_time[0]) ? cycles : min_time[0]);
            max_time[0]  = ((cycles > max_time[0]) ? cycles : max_time[0]); 
            total_time_func += cycles;
            #endif

            // ENCODE - form codeword (repetitions)
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            encoder_repetition_encode(U_K,C_N,info_bits,n_reps);
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[1] += cycles;
            min_time[1]  = ((cycles < min_time[1]) ? cycles : min_time[1]);
            max_time[1]  = ((cycles > max_time[1]) ? cycles : max_time[1]);
            total_time_func += cycles;
            #endif

            // MODULATE - convert to symbol
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            modulate_fn(C_N, X_N, codeword_size);
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[2] += cycles;
            min_time[2]  = ((cycles < min_time[2]) ? cycles : min_time[2]);
            max_time[2]  = ((cycles > max_time[2]) ? cycles : max_time[2]);
            total_time_func += cycles;
            #endif
            
            // CHANNEL - add noise
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            channel_AGWN_add_noise(X_N, Y_N, codeword_size, sigma, rangen);
 	        #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[3] += cycles;
            min_time[3]  = ((cycles < min_time[3]) ? cycles : min_time[0]);
            max_time[3]  = ((cycles > max_time[3]) ? cycles : max_time[0]);
            total_time_func += cycles;
            #endif

            // DEMODULATE - receive from channel
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            demodulate_fn(Y_N, L_N, codeword_size, sigma);
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[4] += cycles;
            min_time[4]  = ((cycles < min_time[4]) ? cycles : min_time[0]);
            max_time[4]  = ((cycles > max_time[4]) ? cycles : max_time[0]);
            total_time_func += cycles;
            #endif

            // DECODE - recover message 

            if (use_fixed) {
                quantizer_transform8(L_N, L8_N, codeword_size, s, f);     //Quantizer
                #ifdef ENABLE_STATS
                begin_step = clock(); // We don't want to take quantizer time into account
                #endif
                decoder_fn_fixed(L8_N, V_K, info_bits, n_reps);
            } 
            else {
                #ifdef ENABLE_STATS
                begin_step = clock();
                #endif
                decoder_fn_float ( L_N, V_K, info_bits, n_reps);
            }
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[5] += cycles;
            min_time[5]  = ((cycles < min_time[5]) ? cycles : min_time[0]);
            max_time[5]  = ((cycles > max_time[5]) ? cycles : max_time[0]);
            total_time_func += cycles;
            #endif

            // MONITOR - error check
            #ifdef ENABLE_STATS
            begin_step = clock();
            #endif
            monitor_fn(U_K, V_K, info_bits, &n_bit_errors, &n_frame_errors);
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[6] += cycles;
            min_time[6]  = ((cycles < min_time[6]) ? cycles : min_time[6]);
            max_time[6]  = ((cycles > max_time[6]) ? cycles : max_time[6]);
            total_time_func += cycles;
            #endif

            n_frame_simulated++;
        } while (n_frame_errors < f_max);

        end_time = clock();
        elapsed = (float) (end_time - start_time)*1000000 / CLOCKS_PER_SEC; // microseconds
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

        fprintf(file_stats,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", val,
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
        fprintf(file, "%f, %f, %f, %lu, %lu, %lu, %f, %f, %f, %f, %f\n", 
            val, SNR_better, sigma,
            n_bit_errors, n_frame_errors, n_frame_simulated,
            ber, fer, elapsed, average, sim_thr);
	    fflush(file);
    }
    gsl_rng_free (rangen);
    return 0;
}

