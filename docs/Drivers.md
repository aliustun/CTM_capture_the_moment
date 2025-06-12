# OV7670 Camera Driver Documentation

---

## Overview

This driver provides initialization and control routines for the OV7670 camera module using STM32F4 microcontrollers. It configures the camera through I2C (SCCB protocol), sets up DCMI for image capture, and manages DMA transfers for efficient data handling.

---

## Features

- I2C initialization and communication with OV7670 camera registers
- DCMI peripheral setup for image data capture
- DMA configuration for circular buffering of camera data
- Clock generation for camera (XCLK) using MCU MCO output
- Camera register initialization with default settings for QVGA RGB565 output format

---

## Supported Hardware Pins

| MCU Pin | Camera Signal |
|---------|----------------|
| PB8     | SIOC (I2C Clock) |
| PB9     | SIOD (I2C Data) |
| PB7          | VSYNC     |
| PA4          | HS        |	
| PA6          | PCLK      |
| PA8          | XCLK      |
| PE6	         | D7        |
| PE5          | D6        |	
|	PD3          | D5        |
| PE4          | D4        |
| PC9          | D3        |
| PC8          | D2        |
| PC7          | D1        |
| PC6          | D0        |

---

## API Reference

### te_CAMERA_ERROR_CODES Camera_Open(void)

Initializes all required peripherals and configures the OV7670 camera module.

*Returns:*  

E_CAMERA_ERR_NONE on success, otherwise an error code indicating the failure reason.

---

## Enumerations

### te_CAMERA_ERROR_CODES

| Enum                   | Description                 |
|------------------------|-----------------------------|
| E_CAMERA_ERR_NONE     | No error                    |
| E_CAMERA_ERR_I2C_INIT| I2C initialization failed  |
| E_CAMERA_ERR_DCMI_INIT| DCMI initialization failed |
| E_CAMERA_ERR_DMA_INIT | DMA initialization failed  |
| E_CAMERA_ERR_CAMERA_INIT | Camera register init failed |

---

## Internal Functions (Static)

These functions are not exposed publicly but form the internal implementation of the driver:

- static te_CAMERA_ERROR_CODES Camera_Init(void)  
  Writes default configuration registers to the OV7670 via I2C.

- static void Camera_GPIO_Init(void)  
  Configures GPIO pins for camera data, control signals, and I2C.

- static te_CAMERA_ERROR_CODES Camera_DCMI_Init(void)  
  Initializes the DCMI peripheral with proper parameters for OV7670 data capture.

- static void Camera_XCLK_Init(void)  
  Sets up the MCU clock output (MCO) to provide the external clock (XCLK) signal to the camera.

- static te_CAMERA_ERROR_CODES Camera_DMA_Init(void)  
  Configures the DMA stream for circular buffer mode linked to DCMI.

- static te_CAMERA_ERROR_CODES Camera_I2C_Init(void)  
  Sets up the I2C peripheral for SCCB communication with OV7670.

---

## Data Structures and Definitions

| Macro / Constant            | Description                              |
|----------------------------|------------------------------------------|
| OV7670_REG_NUM           | Number of camera registers configured (122) |
| OV7670_WRITE_ADDR        | I2C write address for OV7670 (0x42)     |
| IMG_ROWS                 | Image height in pixels (320)             |
| IMG_COLUMNS              | Image width in pixels (240)              |

---

## How It Works

1. *Camera_Open()* is called to start initialization.
2. I2C peripheral is configured for communication.
3. DCMI peripheral is set up to capture data synchronized with camera signals.
4. DMA is configured to transfer image data continuously to memory.
5. Camera external clock (XCLK) is generated using MCU's MCO pin.
6. OV7670 registers are written to via I2C to configure resolution, color format, and other camera parameters.
7. After successful initialization, the camera is ready to capture images.

---

## Usage Example

```c
#include "camera_drv.h"

int main(void) {
    // HAL initialization here

    te_CAMERA_ERROR_CODES status = Camera_Open();
    if (status != E_CAMERA_ERR_NONE) {
        // Handle error
    }

    // Start DCMI capture, DMA, etc.
}
```

# LCD Driver Documentation

This document describes the functions of the SPI-based 320x240 LCD driver for STM32F4.

---

## Overview

- The LCD is controlled via SPI5 interface.
- GPIO pins are configured for LCD control and SPI communication.
- Basic LCD control functions and a generic IOCTL interface are provided.

---

## Functions

### te_LCD_ERROR_CODES LCD_Open(void* vpParam)

*Description:*  

Initializes the LCD driver. Configures GPIO and SPI peripherals, resets the LCD, and performs initial setup.

*Parameters:* 
 
- vpParam: Generic parameter pointer (currently unused).

*Returns:*  

- E_LCD_ERR_NONE: Initialization successful.  
- E_LCD_ERR_SPI_INIT: SPI initialization failed.

---

### te_LCD_ERROR_CODES LCD_Ioctl(te_LCD_IOCTL_COMMANDS eCommand, void * vpParam)

*Description:*  

General control interface for the LCD. Supports operations like drawing a pixel, filling the screen, displaying images, rotating the screen, etc.

*Parameters:*  

- eCommand: The control command to execute (e.g., E_LCD_IOCTL_DRAW_PIXEL).  
- vpParam: Pointer to the data or parameters required by the command.

*Returns:*  

- E_LCD_ERR_NONE: Operation successful.  
- E_LCD_ERR_WRONG_IOCTL_CMD: Unsupported command.

---

### te_LCD_ERROR_CODES LCD_Write(const void *pvBuffer, const uint32_t xBytes)

*Description:* 
 
Writes pixel color data to the currently active window on the LCD.

*Parameters:*
  
- pvBuffer: Pointer to the color data buffer (of type ts_LCD_WR_TYPE).  
- xBytes: Number of bytes to write (not used in current implementation).

*Returns:*  

- E_LCD_ERR_NONE: Write operation successful.

---

### te_LCD_ERROR_CODES LCD_Close(void* vpParam)

*Description:* 
 
Closes the LCD driver, turns off the display, and stops SPI communication.

*Parameters:* 
 
- vpParam: Generic parameter pointer (currently unused).

*Returns:*  

- E_LCD_ERR_NONE: Successfully closed.

---

### static te_LCD_ERROR_CODES LCD_GPIO_Init(void)

*Description:*  

Configures GPIO pins and clocks required for the LCD and SPI interface.

*Returns:*  

- E_LCD_ERR_NONE: GPIO initialization successful.

---

### static te_LCD_ERROR_CODES LCD_SPI_Init(void)

*Description:*  

Initializes SPI5 peripheral as a master with the necessary settings for LCD communication.

*Returns:*  

- E_LCD_ERR_NONE: SPI initialization successful.  
- E_LCD_ERR_SPI_INIT: SPI initialization failed.

---

### void LCD_Reset(void)

*Description:*  
Performs a hardware reset on the LCD by toggling the reset pin.

---


## Error Codes (te_LCD_ERROR_CODES)

| Code                     | Meaning                        |
|--------------------------|-------------------------------|
| E_LCD_ERR_NONE         | Operation successful           |
| E_LCD_ERR_SPI_INIT     | SPI initialization error      |
| E_LCD_ERR_WRONG_IOCTL_CMD | Invalid IOCTL command       |

---

## Usage Example

```c
if (LCD_Open(NULL) != E_LCD_ERR_NONE) {
    // Handle error
}

ts_LCD_DRAW_PIXEL pixel = {x, y, color};
LCD_Ioctl(E_LCD_IOCTL_DRAW_PIXEL, &pixel);

LCD_Close(NULL);
```
