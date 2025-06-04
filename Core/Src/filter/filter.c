#include "filter.h"
#include "camera_drv.h"
#include <string.h>

// Define kernels as file-scope constants
static const int laplacian_kernel[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};

static const int gaussian_kernel[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};

static const int gaussian_factor = 16;

void applyKernel3x3_window(uint8_t window[3][3], const int kernel[3][3], int kernel_factor, int *result) {
    int32_t sum = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sum += window[i][j] * kernel[i][j];
    if (kernel_factor != 0) sum /= kernel_factor;
    if (sum > 255) sum = 255;
    if (sum < 0) sum = 0;
    *result = sum;
}

void applyFilterToImage(uint16_t *input_image, uint16_t *output_image, FilterType filter_type) {
    if (filter_type == FILTER_NONE) {
        // Just copy the middle row of the chunk
        for (int x = 0; x < IMG_COLUMNS; x++) {
            output_image[x] = input_image[IMG_COLUMNS + x];
        }
        return;
    }

    uint8_t window[3][3];
    int result;

    // Process only the middle row of the chunk
    for (int x = 1; x < IMG_COLUMNS-1; x++) {
        // Extract 3x3 window for RGB components
        for (int wy = 0; wy < 3; wy++) {
            for (int wx = 0; wx < 3; wx++) {
                uint16_t rgb = input_image[(wy * IMG_COLUMNS) + (x - 1 + wx)];
                uint8_t r = (rgb >> 11) & 0x1F;
                uint8_t g = (rgb >> 5) & 0x3F;
                uint8_t b = rgb & 0x1F;
                // Convert to grayscale
                window[wy][wx] = (r * 299 + g * 587 + b * 114) / 1000;
            }
        }
        
        // Apply kernel
        if (filter_type == FILTER_LAPLACIAN) {
            applyKernel3x3_window(window, laplacian_kernel, 1, &result);
            // For Laplacian, just use black and white
            result = (result < 128) ? 0 : 0xFFFF;  // Full black or full white
            output_image[x] = result;
        } else {
            applyKernel3x3_window(window, gaussian_kernel, 16, &result);
            // For Gaussian, convert back to RGB565
            uint8_t r5 = (result * 31) / 255;  // 5 bit
            uint8_t g6 = (result * 63) / 255;  // 6 bit
            uint8_t b5 = (result * 31) / 255;  // 5 bit
            output_image[x] = (r5 << 11) | (g6 << 5) | b5;
        }
    }

    // Handle edge pixels
    output_image[0] = input_image[IMG_COLUMNS];  // First pixel
    output_image[IMG_COLUMNS-1] = input_image[IMG_COLUMNS * 2 - 1];  // Last pixel
}

void applyFilterToImageFull(uint16_t *input_image, uint16_t *output_image, FilterType filter_type) {
    if (filter_type == FILTER_NONE) {
        // Filtre yoksa direkt kopyala
        memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
        return;
    }

    // Kernel seçimi
    const int (*kernel)[3] = (filter_type == FILTER_LAPLACIAN) ? laplacian_kernel : gaussian_kernel;
    int kernel_factor = (filter_type == FILTER_LAPLACIAN) ? 1 : gaussian_factor;

    // İlk iki satırı beyaz yap
    for (int i = 0; i < 2 * IMG_COLUMNS; i++) {
        output_image[i] = 0xFFFF;
    }

    // Son iki satırı beyaz yap
    for (int i = (IMG_ROWS - 2) * IMG_COLUMNS; i < IMG_ROWS * IMG_COLUMNS; i++) {
        output_image[i] = 0xFFFF;
    }

    // Kenar pikselleri hariç tüm görüntüyü işle
    for (int row = 1; row < IMG_ROWS - 1; row++) {
        // İşlenen satırın ilk ve son pikselleri beyaz yap
        output_image[row * IMG_COLUMNS] = 0xFFFF;                    // Satırın ilk pikseli
        output_image[row * IMG_COLUMNS + (IMG_COLUMNS - 1)] = 0xFFFF; // Satırın son pikseli

        for (int col = 1; col < IMG_COLUMNS - 1; col++) {
            uint8_t window[3][3];
            
            // 3x3 pencere oluştur
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    uint16_t rgb = input_image[(row + i) * IMG_COLUMNS + (col + j)];
                    uint8_t r = (rgb >> 11) & 0x1F;
                    uint8_t g = (rgb >> 5) & 0x3F;
                    uint8_t b = rgb & 0x1F;
                    uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000;
                    window[i + 1][j + 1] = gray;
                }
            }
            
            // Kernel uygula
            int result;
            applyKernel3x3_window(window, kernel, kernel_factor, &result);
            
            // RGB565 formatına dönüştür
            uint8_t r5 = (result * 31) / 255; // 5 bit
            uint8_t g6 = (result * 63) / 255; // 6 bit
            uint8_t b5 = (result * 31) / 255; // 5 bit
            uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
            
            output_image[row * IMG_COLUMNS + col] = rgb565;
        }
    }
}
