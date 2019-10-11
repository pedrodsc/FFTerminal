#include "fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

double complex *fft_create_lookup(int size){
    double PI = acos(-1);
    
    double complex *lookup = (double complex*) malloc(size*sizeof(double complex));
    
    for (int n = 0; n < size; n++)
        lookup[n] = cexp(-I * 2 * PI * n / size);
    
    return (lookup);
}

void fft_abs(double complex *input,unsigned int *output, int size){
    for (int l = 0; l < size; l++)
        output[l] = (unsigned int) cabs(input[l]);
}

void fft_compute(double complex *lookup, double complex *input, double complex *output, int size){
// Ideia do programa
    int l, half_size = size/2;
    double complex *in_odd, *in_even, *out_odd, *out_even, *local_lookup;
    
    in_even = (double complex*) malloc(sizeof(double complex)*half_size);
    in_odd = (double complex*) malloc(sizeof(double complex)*half_size);
    
    out_even = (double complex*) malloc(sizeof(double complex)*half_size);
    out_odd = (double complex*) malloc(sizeof(double complex)*half_size);
    
    local_lookup = (double complex*) malloc(sizeof(double complex)*half_size);
    
    // Separa input em odd e even
    for (l = 0; l < half_size; l++){
        in_odd[l] = input[2*l+1];
        in_even[l] = input[2*l];
    }
    
    // Divide lookup ao meio
    for (l = 0; l < half_size; l++)
        local_lookup[l] = lookup[2*l];
    
    // Base de recursão com DFT de 2 pontos
//     if (size > 2){
//         fft_compute(local_lookup,in_odd,out_odd,half_size);
//         fft_compute(local_lookup,in_even,out_even,half_size);
//     }else{
//         out_odd[0] = in_odd[0];
//         out_even[0] = in_even[0];
//     }
    // Base de recursão com DFT de 4 pontos
//     if (size > 4){
//         fft_compute(local_lookup,in_odd,out_odd,half_size);
//         fft_compute(local_lookup,in_even,out_even,half_size);
//     }else{
//         out_odd[0] = in_odd[0] + in_odd[1];
//         out_odd[1] = in_odd[0] - in_odd[1];
//         out_even[0] = in_even[0] + in_even[1];
//         out_even[1] = in_even[0] - in_even[1];
//     }
    // Base de recursão com DFT de 2 pontos
    if (size > 8){
        fft_compute(local_lookup,in_odd,out_odd,half_size);
        fft_compute(local_lookup,in_even,out_even,half_size);
    }else{
        out_odd[0] = in_odd[0] + in_odd[1] + in_odd[2] + in_odd[3];
        out_odd[1] = in_odd[0] + I*in_odd[1] - in_odd[2] - I*in_odd[3];
        out_odd[2] = in_odd[0] - in_odd[1] + in_odd[2] - in_odd[3];
        out_odd[3] = in_odd[0] - I*in_odd[1] - in_odd[2] + I*in_odd[3];
        
        out_even[0] = in_even[0] + in_even[1] + in_even[2] + in_even[3];
        out_even[1] = in_even[0] + I*in_even[1] - in_even[2] - I*in_even[3];
        out_even[2] = in_even[0] - in_even[1] + in_even[2] - in_even[3];
        out_even[3] = in_even[0] - I*in_even[1] - in_even[2] + I*in_even[3];
    }
    
    //     output = even + pesos*odd;
    //     return (output);
    for (l = 0; l < half_size; l++){
        output[l] = out_odd[l] + lookup[l]*out_even[l];
        output[half_size+l] = out_odd[l] + lookup[half_size+l]*out_even[l];
    }
    free(in_even);
    free(in_odd);
    free(out_even);
    free(out_odd);
    free(local_lookup);
}

void fft128_compute(double complex *lookup, double complex *input, double complex *output){
    //     Ideia pra o iterativo
    // 128
    
    // 64
    
    // 32
    
    // 16
    
    // 8
    
    // 4
    
    // 2
    //     y0 = x0 + x1
    //     y1 = x0 - x1
}
