#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

typedef enum {
    FILTER_NONE,
    FILTER_LAPLACIAN,
    FILTER_GAUSSIAN
} FilterType;

extern const int laplacian_kernel[3][3];
extern const int gaussian_kernel[3][3];
extern const int gaussian_factor;

void applyKernel3x3_window(uint8_t window[3][3], const int kernel[3][3], int kernel_factor, int *result);

#endif // FILTER_H
