#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <math.h>

#define LOOKUP_SIZE          2048

double complex *fft_create_lookup(int size);
void fft_abs(double complex *input,unsigned int *output, int size);
void fft_compute(double complex *lookup, double complex *input, double complex *output, int size);
void fft128_compute(double complex *lookup, double complex *input, double complex *output);

#endif

