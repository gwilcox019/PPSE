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
#include <arm_neon.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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
    vst1q_u8(T, vector);
    for (int i=0; i<16; i++) {
        printf("%i ; ", T[i]);
    }
}
void display_int8x16 (int8x16_t vector) {
    int8_t T[16];
    vst1q_u8((uint8_t *)T, (uint8x16_t)vector);
    for (int i=0; i<16; i++) {
        printf("%i ; ", T[i]);
    }
}

// generates random frame of k bits
//write into the buffer U_K
void source_generate(uint8_t* UK, size_t k) {
    for (; k>0; k--)
        UK[k-1] = rand()%2 ;      // Returns a pseudo-random integer between 0 and RAND_MAX.
}

// alternative generator with all zero
void source_generate_all_zeros(uint8_t *U_K, size_t K) {
    for (; K>0; K--)
        U_K[K-1] = 0 ; 
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
    for (; N>0; N--)
        X_N[N-1] = 1;
}

// adds random noise following a normal distribution
void channel_AGWN_add_noise(const int32_t* X_N, float* Y_N, size_t n, float sigma, gsl_rng * rangen) {
    for (; n>0; n--) {
        float v = gsl_ran_gaussian(rangen, sigma); // calculates normal value with sigma from uniform random number
        Y_N[n-1] = X_N[n-1] + v;
    }

}

void modem_BPSK_demodulate (const float* Y_N, float* L_N, size_t n, float sigma){
        float constant = 2.0f/(sigma*sigma);
	for (int i=0; i<n; i++) {
		L_N[i] = Y_N[i] * constant; 
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

// for detecting overflow
int8_t sat_int_add(int8_t a, int8_t b) {
    int8_t c = a+b;
    if (a<0 && b<0) return c<0 ? c : 0x80;
    if (a>0 && b>0) return c>0 ? c : 0x7F;
    return c;
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

void codec_repetition_hard_decode8_neon(const int8_t *L8_N, uint8_t *V_K, size_t K, size_t n_reps) {
    // truc intermédiaire pour charger la valeur
    int array_nb = K/16;
    //print_array8(L8_N, K*n_reps);
    int8x16_t decomposed; //Stores the raw data that we are working on
    int8x16_t sum; //Stores the local sum for that part of the array
    int8x16_t only1 = vdupq_n_s8(1);
    for (int a=0; a<array_nb; a++) { //Pour chaque groupe de 16 valeurs (taille de K)
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

    char use_fixed = 0;
    int f = 8;
    int s = 1;

    void  (*decoder_fn_float) (const float*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode;
    void  (*decoder_fn_fixed) (const int8_t*, uint8_t *, size_t, size_t) = codec_repetition_soft_decode8;
    void (*generate_fn) (uint8_t*, size_t) = source_generate;
    void (*modulate_fn) (const uint8_t*, int32_t*, size_t) = module_bpsk_modulate;
    char filepath[20] = {0};
    char filepath_stats[30] = {0};
    // For long option - we set an alias
    struct option zero_opt[5] = {{"src-all-zeros", no_argument, NULL, 'z'}, 
                                {"mod-all-ones", no_argument, NULL, 'o'},
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
    while ((opt = getopt_long(argc, argv, "m:M:s:e:K:N:D:f:", zero_opt, &long_index)) != -1) {
        switch(opt) {
            case 'm': min_SNR = atof(optarg); break;
            case 'M': if ((max_SNR = atof(optarg)) == 0) max_SNR = 12; break;
            case 's': if ((step_val = atof(optarg)) == 0) step_val = 1; break;
            case 'e': if ((f_max = atoi(optarg)) == 0) f_max = 100; break;
            case 'K': if ((info_bits = atoi(optarg)) == 0) info_bits = 32; break;
            case 'N': if ((codeword_size = atoi(optarg)) == 0) codeword_size = 128; break;
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
            case 'z': //Check long
                generate_fn = source_generate_all_zeros;
                break;
            case 'o': //check long
                modulate_fn = modem_BPSK_modulate_all_ones; 
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

    // blocks used :          source gen, encode,        modulate,      channel,       demodulate,    decode,    monitor
    uint32_t block_bits[7] = {info_bits,  codeword_size, codeword_size, codeword_size, codeword_size, info_bits, info_bits}; // number of bits for each block

    // Stats - computed after one loop
    float ber, fer;

    // Output file
    FILE* file;
    if (filepath[0] == 0) file = stdout;
    else file = fopen(filepath, "w");
    FILE* file_stats;
    if (filepath_stats[0] == 0) file_stats = stdout;
    else file_stats = fopen(filepath_stats, "w");
    fprintf(file, "Eb/No,Es/No,Sigma,# Bit Errors,# Frame Errors,# Simulated frames,BER,FER,Time for this SNR,Average time for one frame,SNR throughput\n");
    fprintf(file_stats, "Eb/No,gen_avg,gen_min,gen_max,gen_thr,gen_percent,encode_avg,encode_min,encode_max,encode_thr,encode_percent,bpsk_avg,bpsk_min,bpsk_max,bpsk_thr,bpsk_percent,awgn_avg,awgn_min,awgn_max,awgn_thr,awgn_percent,demodulate_avg,demodulate_min_demodulate_max,demodulate_thr,demodulate_percent,decode_avg,decode_min,decode_max,decode_thr,decode_percent,monitor_avg,monitor_min,monitor_max,monitor_thr,monitor_percent\n");
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
    float cycles = 0;
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
	for (int i=0; i<7; i++) {
		min_time[i] = INFINITY;
		max_time[i] = -INFINITY;
		avg_time[i] = 0;
	}
        #endif

        SNR_better = val + 10*log10f(R); // have to use log10 not just log
        sigma = sqrt( 1 / (2 * pow(10, (SNR_better/10) ) ) ); 

        printf("current snr = %f, min snr = %f, max snr = %f, sigma = %f\n", min_SNR, max_SNR, val, sigma);

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
            modem_BPSK_demodulate(Y_N, L_N, codeword_size, sigma);
            #ifdef ENABLE_STATS
            end_step = clock(); 
            cycles = ((end_step-begin_step)*1000000)/CLOCKS_PER_SEC;
            avg_time[4] += cycles;
            min_time[4]  = ((cycles < min_time[4]) ? cycles : min_time[0]);
            max_time[4]  = ((cycles > max_time[4]) ? cycles : max_time[0]);
            total_time_func += cycles;
            #endif

            // DECODE - recover message 
            #ifdef ENABLE_STATS
             begin_step = clock();
            #endif
            if (use_fixed) {
                quantizer_transform8(L_N, L8_N, codeword_size, s, f);                //Quantizer
                decoder_fn_fixed(L8_N, V_K, info_bits, n_reps);
            } else decoder_fn_float ( L_N, V_K, info_bits, n_reps);
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
            monitor_check_errors(U_K, V_K, info_bits, &n_bit_errors, &n_frame_errors);
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
        fprintf(file, "%f, %f, %f, %li, %li, %li, %f, %f, %f, %f, %f\n", 
            val, SNR_better, sigma,
            n_bit_errors, n_frame_errors, n_frame_simulated,
            ber, fer, elapsed, average, sim_thr);
	fflush(file);
    }
    gsl_rng_free (rangen);
    return 0;
}
/*
int main() {

    //Init random
    srand(time(NULL));   // Initialization, should only be called once.
    const gsl_rng_type * rangentype;
    rangentype = gsl_rng_default;
    gsl_rng * rangen = gsl_rng_alloc (rangentype); // random number gen w uniform distr 

    size_t K = 32, N = 8192, REPS = 256;
    uint8_t UK[N], CN[N];
    int32_t XN[N];
    float YN[N], LN[N];
    int8_t L8N[N];
    uint8_t VN[K];

    for (int i = 0; i < 1; i++)
    {
        // float sigma=0;
        //  Generate message
        source_generate(UK, K);
        printf("\nTableau genere : ");
        print_array(UK, K);

        // Encoded message
        encoder_repetition_encode(UK, CN, K, REPS);
        //printf("\nTableau encode : ");
        //print_array(CN, N);

        // Modulated message
        module_bpsk_modulate(CN, XN, N);
        //printf("\nTableau module : ");
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
        quantizer_transform8(LN, L8N, N, 5, 3);

        printf("Tableau hard dec : \n");
        codec_repetition_hard_decode8_neon(L8N, VN, K, REPS);
        printf("\n");

        printf("Tableau hard dec NORMAL: \n");
        codec_repetition_hard_decode(LN, VN, K, REPS);
	print_array(VN, K);
        printf("\n");

        printf("Tableau hard dec fixed: \n");
        codec_repetition_hard_decode8(L8N, VN, K, REPS);
	print_array8(VN, K);
        printf("\n");

    }

    gsl_rng_free(rangen);
    return 0;
}
*/
