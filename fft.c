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

void fft_compute(double complex *lookup, double complex *input, double complex *output, int size){
    for (int n = 0; n < size; n++)
        output[n] = lookup[n] + 2*input[n];
}
