#include "debug_func.h"

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
