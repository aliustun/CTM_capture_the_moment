#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>
#include <stdbool.h>

// ROI optimizasyon ayarları
#define ROI_OPT_BLOCK_SIZE 32  // ROI blok boyutu
#define ROI_OPT_THRESHOLD 0   // Değişim algılama eşiği

#define ROI_TH 0
#define ROI_WIDTH 5
#define ROI_HEIGHT 5

// Merkez ROI Alarm için sabitler
#define CENTER_ROI_SIZE 50
#define CENTER_ROI_TH 25000
#define ALARM_COLOR 0xF800  // Kırmızı renk (RGB565 formatında)
#define ALARM_DURATION_MS 1000 // Alarm süresi (ms)

typedef enum {
    FILTER_NONE,
    FILTER_LAPLACIAN,
    FILTER_GAUSSIAN,
    FILTER_GRAYSCALE,
    FILTER_ROI,
    FILTER_ROI_CENTER_ALARM
} FilterType;

extern const int laplacian_kernel[3][3];
extern const int gaussian_kernel[3][3];
extern const int gaussian_factor;

void applyKernel3x3_window(uint8_t window[3][3], const int kernel[3][3], int kernel_factor, int *result);
void applyFilterToImage(uint16_t *input_image, uint16_t *output_image, FilterType filter_type);
void applyFilterToImageFull(uint16_t *input_image, uint16_t *output_image, FilterType filter_type);

// Filtre testlerini çalıştıran ana fonksiyon
void runFilterTests(void);
static void testCenterROIAlarm(void);
static void createTestImage(uint16_t *image, int pattern);
static int compareImages(uint16_t *img1, uint16_t *img2, int size);
static void testGrayscaleFilter(void);
static void testLaplacianFilter(void);
static void testROIOptimization(void);
static void testCenterROIAlarm(void);
// ROI optimizasyon kontrolü için fonksiyonlar
void setROIOptimizationEnabled(bool enabled);
bool isROIOptimizationEnabled(void);

#endif // FILTER_H
