#include "filter.h"

const int laplacian_kernel[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};
const int gaussian_kernel[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};
const int gaussian_factor = 16;

void applyKernel3x3_window(uint8_t window[3][3], const int kernel[3][3], int kernel_factor, int *result) {
    int sum = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sum += window[i][j] * kernel[i][j];
    if (kernel_factor != 0) sum /= kernel_factor;
    if (sum > 255) sum = 255;
    if (sum < 0) sum = 0;
    *result = sum;
}
