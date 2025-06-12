# LCD Driver

The LCD driver provides low-level control over a TFT LCD screen connected via SPI. It includes GPIO and SPI initialization, display reset, and command/data communication using the ILI9341 controller protocol. 

Core functionalities include:

- Initialization sequence for power control, VCOM, gamma, and pixel format settings
- Drawing individual pixels or entire images to the display memory (GRAM)
- Screen rotation support in 4 directions
- IOCTL-style interface for issuing commands such as filling the screen, drawing pixels, or setting rotation
- Efficient image transfer using buffered SPI communication

The driver abstracts all hardware control so that higher-level modules can render images or UI elements easily on the screen.

---

# Camera Driver

The camera driver handles configuration and communication with the OV7670 camera module using I2C (SCCB) and DCMI (camera interface) with DMA support for efficient image capture.

Main responsibilities:

- Hardware initialization for GPIOs, I2C, DCMI, and DMA
- Reset and setup of camera registers for QVGA resolution and RGB565 color mode
- Frame acquisition using DMA into memory
- Exposure and white balance configuration via OV7670 register presets
- Integration with the LCD driver to preview the camera output on-screen

It offers an easy-to-use interface to start the camera and stream video frames into memory with minimal CPU overhead.
