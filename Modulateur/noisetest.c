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

void print_array_float (float* array, size_t size) {
    printf("[");
    for (int i=0; i<size; i++) {
        printf("%f ; ", array[i]);
    }
    printf("]");
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

int main( int argc, char** argv) {
    size_t n = 4;
    int32_t X_N[] = {-1,1,1,-1};
    float Y_N[n];
    float sigma = 0.1;
    channel_AGWN_add_noise(X_N,Y_N,n,sigma);
    print_array_float(Y_N, n);
}