
# Image Filtering Module – Documentation

## Overview

This module provides a set of image filtering functions for use in embedded systems, particularly in ARM Cortex-M based platforms using FreeRTOS. It includes support for:

- Grayscale conversion
- Laplacian and Gaussian filters
- Region of Interest (ROI) detection
- ROI-based alarm system for motion detection

Memory-mapped SDRAM is used to store previous frames for differential processing. The module is optimized to work with RGB565 formatted images and leverages FreeRTOS time management for alarm timing.

---

## Dependencies

```c
#include "filter.h"
#include "camera_drv.h"
#include "cmsis_os.h"
```

- `filter.h`: Filter type definitions and image size macros (e.g., `IMG_ROWS`, `IMG_COLUMNS`)
- `camera_drv.h`: Camera-related constants and functions
- `cmsis_os.h`: Required for FreeRTOS kernel tick functions

---

## Constants

### SDRAM Memory Mapping

```c
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000)
#define PREVIOUS_FRAME_ADDR (SDRAM_BANK_ADDR + 0x100000)
```

- Used to store the previous frame for ROI comparison.
- Ensures non-volatile temporary storage across frames.

### Kernel Definitions

```c
const int laplacian_kernel[3][3] = { {-1,-1,-1}, {-1,8,-1}, {-1,-1,-1} };
const int gaussian_kernel[3][3] = { {1,2,1}, {2,4,2}, {1,2,1} };
const int gaussian_factor = 16;
```

- 3x3 convolution kernels for edge detection and noise reduction.

---

## Filter Types (`FilterType`)

This enum (from `filter.h`) defines the available image filters:
- `FILTER_NONE`: No filtering
- `FILTER_GRAYSCALE`: Converts RGB565 to grayscale
- `FILTER_LAPLACIAN`: Applies a Laplacian filter
- `FILTER_GAUSSIAN`: Applies a Gaussian blur
- `FILTER_ROI`: Highlights changed regions using previous frame
- `FILTER_ROI_CENTER_ALARM`: Triggers alarm on central movement

---

## Function Descriptions

### `void applyKernel3x3_window(...)`

Applies a 3x3 kernel to a grayscale window.

- **Inputs**:
  - `window[3][3]`: Grayscale values
  - `kernel[3][3]`: Convolution kernel
  - `kernel_factor`: Normalization factor
- **Output**:
  - `*result`: Resulting pixel value after kernel application

---

### `void applyFilterToImage(...)`

Applies a specified filter line-by-line with limited memory usage.

- Optimized for memory-constrained environments
- Maintains 3-line rolling buffer for kernel filtering
- Skips kernel on the first two lines (fills black)

---

### `void applyFilterToImageFull(...)`

Applies a specified filter to the entire image frame.

#### `FILTER_NONE`:
- Simple copy of `input_image` to `output_image`

#### `FILTER_GRAYSCALE`:
- Converts each pixel to grayscale using luminance approximation:
  ```c
  gray = (r * 299 + g * 587 + b * 114) / 1000;
  ```

#### `FILTER_LAPLACIAN` / `FILTER_GAUSSIAN`:
- Applies respective 3x3 kernel across the image (ignores edges)

#### `FILTER_ROI`:
- Compares current frame with stored `previous_frame`
- Highlights changed 5x5 regions if grayscale difference exceeds `ROI_TH`
- Region size: `ROI_WIDTH`, `ROI_HEIGHT`
- Retains only moving parts

#### `FILTER_ROI_CENTER_ALARM`:
- Monitors central 50x50 pixel area
- Triggers red-screen alarm (`ALARM_COLOR`, RGB565) if motion exceeds `CENTER_ROI_TH`
- Alarm persists for `ALARM_DURATION_MS`

---

## Alarm Logic

### Global Variables

```c
static uint8_t first_frame = 1;
static uint8_t first_frame_center = 1;
static uint32_t alarm_start_time = 0;
static uint8_t alarm_active = 0;
```

- Track whether the frame is the first processed frame
- Manage alarm state and duration

### Alarm Workflow
1. If the center ROI detects motion beyond a threshold:
   - All pixels turn red (`ALARM_COLOR`)
   - Alarm stays active for `ALARM_DURATION_MS`
2. If alarm is still active:
   - The central area remains red regardless of motion
3. After duration expires:
   - Frame returns to normal processing

---

## RGB565 ↔ Grayscale Conversion

### RGB565 to Grayscale

```c
r = (rgb >> 11) & 0x1F;
g = (rgb >> 5) & 0x3F;
b = rgb & 0x1F;
gray = (r * 299 + g * 587 + b * 114) / 1000;
```

### Grayscale to RGB565

```c
r5 = (gray * 31) / 255;
g6 = (gray * 63) / 255;
b5 = (gray * 31) / 255;
rgb565 = (r5 << 11) | (g6 << 5) | b5;
```

---

## Optimization Notes

- Uses 3-line rolling buffer for memory efficiency in `applyFilterToImage`
- Skips processing of edge pixels to avoid out-of-bounds errors
- SDRAM provides persistent frame memory across FreeRTOS tasks
- Central ROI logic optimized with early exit (`break`) for faster decision making

---

## Use Cases

- Embedded image recognition pipelines
- Real-time surveillance systems (motion detection)
- Memory-constrained MCU-based image processors

---

## TODO / Improvements

- Parameterize ROI sizes and thresholds dynamically
- Extend filter support (e.g., Sobel, Median)
- Add interrupt-based frame update handling
