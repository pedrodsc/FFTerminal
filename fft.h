#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <math.h>

double complex *fft_create_lookup(int size);
void fft_compute(double complex *lookup, double complex *input, double complex *output, int size);

#endif

