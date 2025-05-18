# Project Hardware Details
- [STM32F429 Discovery Board](https://www.st.com/en/evaluation-tools/32f429idiscovery.html)
  - Built-in Touchscreen LCD
- [OV7670 Camera Module](https://www.robotsepeti.com/arduino-kamera-modulu-ov7670)

## Connections

### DCMI[^1] Wiring

| **uC Board** | **OV7670**| 
|--------------|-----------|
| PB7          | VSYNC     |
| PA4          | HS        |	
| PA6          | PCLK      |
| PA8          | XCLK      |
| PE6	         | D7        |
| PE5          | D6        |	
|	PB6          | D5        |
| PE4          | D4        |
| PC9          | D3        |
| PC8          | D2        |
| PC7          | D1        |
| PC6          | D0        |

### SSCB[^2] Wiring

| **uC Board** | **OV7670**| 
|--------------|-----------|
| PB8          | SCL       |
| PB9          | SDA       |


[^1]: Digital Camera Interface
[^2]: Serial Camera Control Bus
