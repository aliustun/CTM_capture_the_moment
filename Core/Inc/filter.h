
#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

typedef enum {
    FILTER_LAPLACIAN,
    FILTER_GAUSSIAN
} FilterType;

void applyFilter(uint8_t *input, uint8_t *output, FilterType filter);

#endif // FILTER_H