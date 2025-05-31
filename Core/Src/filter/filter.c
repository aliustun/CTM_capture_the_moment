#include "stm32f4xx_hal.h"
// #include "FreeRTOS.h"
// #include "task.h"
#include "filter.h"

#define WIDTH 640
#define HEIGHT 480

uint8_t image_data[WIDTH * HEIGHT];
uint8_t processed_image[WIDTH * HEIGHT];

// Laplacian Kernel (Edge Detection)
int laplacian_kernel[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};

// Gaussian Kernel (Blur)
int gaussian_kernel[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};
const int gaussian_factor = 16;


// Filtre uygulama fonksiyonu
void applyFilter(uint8_t *input, uint8_t *output, FilterType filter) {
    int x, y, i, j;
    int sum;

    for (y = 1; y < HEIGHT - 1; y++) {
        for (x = 1; x < WIDTH - 1; x++) {
            sum = 0;

            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    int pixel = input[(y + i) * WIDTH + (x + j)];

                    if (filter == FILTER_LAPLACIAN) {
                        sum += laplacian_kernel[i + 1][j + 1] * pixel;
                    } else if (filter == FILTER_GAUSSIAN) {
                        sum += gaussian_kernel[i + 1][j + 1] * pixel;
                    }
                }
            }

            // Normalize ve sınırla
            if (filter == FILTER_GAUSSIAN) {
                sum /= gaussian_factor;
            }

            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;

            output[y * WIDTH + x] = (uint8_t)sum;
        }
    }
}

// Dışarıdan gelecek filtre tipi örneği (simülasyon amaçlı global değişken)
// FilterType selectedFilter = FILTER_LAPLACIAN;

// void captureImageTask(void *pvParameters) {
//     // Kamera verisi alma kodu burada olacak (DMA vs)
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

// void displayTask(void *pvParameters) {
//     while (1) {
//         applyFilter(image_data, processed_image, selectedFilter);

//         // İşlenmiş görüntüyü ekrana yazdırma fonksiyonu çağrılır
//         // displayImage(processed_image);

//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// int main(void) {
//     HAL_Init();

//     // Filtre seçimi örneği: Dışarıdan alınabilir (örneğin UART, buton, vs)
//     // selectedFilter = FILTER_GAUSSIAN; // veya FILTER_LAPLACIAN

//     xTaskCreate(captureImageTask, "Capture Image", 2048, NULL, 2, NULL);
//     xTaskCreate(displayTask, "Display", 2048, NULL, 2, NULL);

//     vTaskStartScheduler();

//     while (1) {
//     }
// }
