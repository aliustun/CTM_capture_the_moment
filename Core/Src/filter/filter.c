#include "filter.h"
#include "camera_drv.h"
#include "cmsis_os.h"  // FreeRTOS için gerekli

// SDRAM adresi (main.c'den alındı)
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)
#define PREVIOUS_FRAME_ADDR (SDRAM_BANK_ADDR + 0x100000) // SDRAM'de önceki kare için ayrılan alan

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

// ROI filtresi için sabitler
#define ROI_TH 0
#define ROI_WIDTH 5
#define ROI_HEIGHT 5

// Merkez ROI Alarm için sabitler
#define CENTER_ROI_SIZE 50
#define CENTER_ROI_TH 25000
#define ALARM_COLOR 0xF800  // Kırmızı renk (RGB565 formatında)
#define ALARM_DURATION_MS 1000 // Alarm süresi (ms)

static uint8_t first_frame = 1; // İlk kare kontrolü
static uint8_t first_frame_center = 1; // Merkez ROI için ilk kare kontrolü
static uint32_t alarm_start_time = 0; // Alarm başlangıç zamanı
static uint8_t alarm_active = 0; // Alarm durumu

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
    // Geçici buffer için 3 satırlık alan
    uint16_t line_buffer[3][IMG_COLUMNS];
    
    // Her satır için işlem yap
    for (int row = 0; row < IMG_ROWS; row++) {
        // Satırı line_buffer'a kopyala
        memcpy(line_buffer[row % 3], &input_image[row * IMG_COLUMNS], IMG_COLUMNS * sizeof(uint16_t));
        
        if (filter_type == FILTER_NONE) {
            // Filtre yoksa direkt kopyala
            memcpy(&output_image[row * IMG_COLUMNS], line_buffer[row % 3], IMG_COLUMNS * sizeof(uint16_t));
        } else if (filter_type == FILTER_GRAYSCALE) {
            // Grayscale dönüşümü
            for (int x = 0; x < IMG_COLUMNS; x++) {
                uint16_t rgb = line_buffer[row % 3][x];
                uint8_t r = (rgb >> 11) & 0x1F;
                uint8_t g = (rgb >> 5) & 0x3F;
                uint8_t b = rgb & 0x1F;
                
                // RGB565'den 8-bit grayscale'e dönüşüm
                uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000;
                
                // Grayscale değeri RGB565 formatına geri dönüştür
                uint8_t r5 = (gray * 31) / 255; // 5 bit
                uint8_t g6 = (gray * 63) / 255; // 6 bit
                uint8_t b5 = (gray * 31) / 255; // 5 bit
                uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
                
                output_image[row * IMG_COLUMNS + x] = rgb565;
            }
        } else if (row >= 2) {
            // Filtre uygulanabilir satırlar için
            int y = row - 1;
            const int (*kernel)[3] = (filter_type == FILTER_LAPLACIAN) ? laplacian_kernel : gaussian_kernel;
            int kernel_factor = (filter_type == FILTER_LAPLACIAN) ? 1 : gaussian_factor;
            
            // Kenar pikselleri hariç işlem yap
            for (int x = 1; x < IMG_COLUMNS - 1; x++) {
                uint8_t window[3][3];
                
                // 3x3 pencere oluştur
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        uint16_t rgb = line_buffer[(row - 1 + i + 3) % 3][x + j];
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
                
                output_image[y * IMG_COLUMNS + x] = rgb565;
            }
        } else {
            // İlk iki satır için siyah yap
            for (int i = 0; i < IMG_COLUMNS; i++) {
                output_image[row * IMG_COLUMNS + i] = 0x0000;
            }
        }
    }
}

void applyFilterToImageFull(uint16_t *input_image, uint16_t *output_image, FilterType filter_type) {
    if (filter_type == FILTER_NONE) {
        // Filtre yoksa direkt kopyala
        memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
        return;
    }

    if (filter_type == FILTER_GRAYSCALE) {
        // Grayscale dönüşümü
        for (int i = 0; i < IMG_ROWS * IMG_COLUMNS; i++) {
            uint16_t rgb = input_image[i];
            uint8_t r = (rgb >> 11) & 0x1F;
            uint8_t g = (rgb >> 5) & 0x3F;
            uint8_t b = rgb & 0x1F;
            
            // RGB565'den 8-bit grayscale'e dönüşüm
            uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000;
            
            // Grayscale değeri RGB565 formatına geri dönüştür
            uint8_t r5 = (gray * 31) / 255; // 5 bit
            uint8_t g6 = (gray * 63) / 255; // 6 bit
            uint8_t b5 = (gray * 31) / 255; // 5 bit
            uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
            
            output_image[i] = rgb565;
        }
        return;
    }

    if (filter_type == FILTER_ROI) {
        uint16_t *previous_frame = (uint16_t *)PREVIOUS_FRAME_ADDR;

        // İlk kareyi işle
        if (first_frame) {
            memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
            memcpy(previous_frame, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
            first_frame = 0;
            return;
        }

        // Çıkış görüntüsünü siyah yap
       // memset(output_image, 0, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));

        // Her ROI_WIDTH ve ROI_HEIGHT piksel için kontrol yap
        for (int y = ROI_HEIGHT; y < IMG_ROWS; y += ROI_HEIGHT) {
            for (int x = ROI_WIDTH; x < IMG_COLUMNS; x += ROI_WIDTH) {
                // Mevcut ve önceki pikselleri al
                uint16_t current_rgb = input_image[y * IMG_COLUMNS + x];
                uint16_t previous_rgb = previous_frame[y * IMG_COLUMNS + x];

                // RGB565'den grayscale'e dönüştür
                uint8_t current_gray = ((current_rgb >> 11) & 0x1F) * 299/1000 + 
                                     ((current_rgb >> 5) & 0x3F) * 587/1000 + 
                                     (current_rgb & 0x1F) * 114/1000;
                
                uint8_t previous_gray = ((previous_rgb >> 11) & 0x1F) * 299/1000 + 
                                      ((previous_rgb >> 5) & 0x3F) * 587/1000 + 
                                      (previous_rgb & 0x1F) * 114/1000;

                // XOR işlemi ve eşik kontrolü
                uint8_t xor_result = current_gray ^ previous_gray;
                if (xor_result > ROI_TH) {
                    // ROI bölgesini kopyala
                    for (int k = -ROI_HEIGHT; k < ROI_HEIGHT; k++) {
                        for (int j = -ROI_WIDTH; j < ROI_WIDTH; j++) {
                            int new_y = y + k;
                            int new_x = x + j;
                            
                            // Sınırları kontrol et
                            if (new_y >= 0 && new_y < IMG_ROWS && new_x >= 0 && new_x < IMG_COLUMNS) {
                                output_image[new_y * IMG_COLUMNS + new_x] = 
                                    input_image[new_y * IMG_COLUMNS + new_x];
                            }
                        }
                    }
                }
            }
        }

        // Mevcut kareyi önceki kare olarak sakla
        memcpy(previous_frame, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
        return;
    }

    if (filter_type == FILTER_ROI_CENTER_ALARM) {
        uint16_t *previous_frame = (uint16_t *)PREVIOUS_FRAME_ADDR;
        uint32_t current_time = osKernelGetTickCount(); // Mevcut zaman

        // İlk kareyi işle
        if (first_frame_center) {
            memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
            memcpy(previous_frame, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
            first_frame_center = 0;
            return;
        }

        // Önce mevcut görüntüyü kopyala
        memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));

        // Eğer alarm aktifse ve süresi dolmamışsa, kırmızı ekranı göstermeye devam et
        if (alarm_active) {
            if ((current_time - alarm_start_time) < (ALARM_DURATION_MS / portTICK_PERIOD_MS)) {
                // Önce mevcut görüntüyü kopyala
                memcpy(output_image, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
                
                // Sadece merkez ROI bölgesini kırmızı yap
                int center_x = IMG_COLUMNS / 2;
                int center_y = IMG_ROWS / 2;
                int roi_start_x = center_x - CENTER_ROI_SIZE/2;
                int roi_start_y = center_y - CENTER_ROI_SIZE/2;
                
                for (int y = roi_start_y; y < roi_start_y + CENTER_ROI_SIZE && y < IMG_ROWS; y++) {
                    for (int x = roi_start_x; x < roi_start_x + CENTER_ROI_SIZE && x < IMG_COLUMNS; x++) {
                        if (y >= 0 && x >= 0) {
                            output_image[y * IMG_COLUMNS + x] = ALARM_COLOR;
                        }
                    }
                }
                return;
            } else {
                // Alarm süresi doldu
                alarm_active = 0;
            }
        }

        // Merkez koordinatları hesapla
        int center_x = IMG_COLUMNS / 2;
        int center_y = IMG_ROWS / 2;
        int roi_start_x = center_x - CENTER_ROI_SIZE/2;
        int roi_start_y = center_y - CENTER_ROI_SIZE/2;

        // Merkez bölgedeki değişimi kontrol et
        int total_change = 0;
        int pixel_count = 0;

        for (int y = roi_start_y; y < roi_start_y + CENTER_ROI_SIZE && y < IMG_ROWS; y++) {
            for (int x = roi_start_x; x < roi_start_x + CENTER_ROI_SIZE && x < IMG_COLUMNS; x++) {
                if (y >= 0 && x >= 0) {  // Negatif indeksleri kontrol et
                    // Mevcut ve önceki pikselleri al
                    uint16_t current_rgb = input_image[y * IMG_COLUMNS + x];
                    uint16_t previous_rgb = previous_frame[y * IMG_COLUMNS + x];

                    // RGB565'den grayscale'e dönüştür
                    uint8_t current_gray = ((current_rgb >> 11) & 0x1F) * 299/1000 + 
                                         ((current_rgb >> 5) & 0x3F) * 587/1000 + 
                                         (current_rgb & 0x1F) * 114/1000;
                    
                    uint8_t previous_gray = ((previous_rgb >> 11) & 0x1F) * 299/1000 + 
                                          ((previous_rgb >> 5) & 0x3F) * 587/1000 + 
                                          (previous_rgb & 0x1F) * 114/1000;

                    // Değişim miktarını hesapla
                    total_change += (current_gray ^ previous_gray);
                    

                }
                if (total_change > CENTER_ROI_TH){
                        break;}
            }
            if (total_change > CENTER_ROI_TH){
                        break;}
        }

        // Ortalama değişimi hesapla
        float average_change = total_change;

        // Eğer değişim eşiği geçerse, alarmı başlat
        if (average_change > CENTER_ROI_TH) {
            alarm_active = 1;
            alarm_start_time = current_time;
            for (int i = 0; i < IMG_ROWS * IMG_COLUMNS; i++) {
                output_image[i] = ALARM_COLOR;
            }
        }

        // Mevcut kareyi önceki kare olarak sakla
        memcpy(previous_frame, input_image, IMG_ROWS * IMG_COLUMNS * sizeof(uint16_t));
        return;
    }

    // Kernel seçimi
    const int (*kernel)[3] = (filter_type == FILTER_LAPLACIAN) ? laplacian_kernel : gaussian_kernel;
    int kernel_factor = (filter_type == FILTER_LAPLACIAN) ? 1 : gaussian_factor;

    // İlk iki satırı siyah yap
    for (int i = 0; i < 2 * IMG_COLUMNS; i++) {
        output_image[i] = 0x0000;
    }

    // Son iki satırı siyah yap
    for (int i = (IMG_ROWS - 2) * IMG_COLUMNS; i < IMG_ROWS * IMG_COLUMNS; i++) {
        output_image[i] = 0x0000;
    }

    // Kenar pikselleri hariç tüm görüntüyü işle
    for (int row = 1; row < IMG_ROWS - 1; row++) {
        // İşlenen satırın ilk ve son pikselleri siyah yap
        output_image[row * IMG_COLUMNS] = 0x0000;                    // Satırın ilk pikseli
        output_image[row * IMG_COLUMNS + (IMG_COLUMNS - 1)] = 0x0000; // Satırın son pikseli

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
