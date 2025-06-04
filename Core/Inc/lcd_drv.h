/*
 * lcd_drv.h
 *
 *  Created on: May 23, 2025
 *      Author: OVatanse
 */

#ifndef INC_LCD_DRV_H_
#define INC_LCD_DRV_H_

#include "stm32f4xx.h"

#define LCD_WIDTH	320
#define LCD_HEIGHT	240

typedef enum {
	E_LCD_ERR_NONE,
	E_LCD_ERR_SPI_INIT
} te_LCD_ERROR_CODES;


te_LCD_ERROR_CODES LCD_Open(void);
void LCD_Fill_Screen(uint16_t color);
void LCD_Display_Image(uint16_t image[LCD_WIDTH * LCD_HEIGHT]);
void LCD_Set_Rotation(uint8_t rotation);

// New function to display image in parts
void LCD_Display_New(uint16_t *image, uint32_t start_index, uint32_t length);

// SPI LCD Commands
#define LCD_RESET			0x01
#define LCD_SLEEP_OUT		0x11
#define LCD_GAMMA			0x26
#define LCD_DISPLAY_OFF		0x28
#define LCD_DISPLAY_ON		0x29
#define LCD_COLUMN_ADDR		0x2A
#define LCD_PAGE_ADDR		0x2B
#define LCD_GRAM			0x2C
#define LCD_MAC				0x36
#define LCD_PIXEL_FORMAT	0x3A
#define LCD_WDB				0x51
#define LCD_WCD				0x53
#define LCD_RGB_INTERFACE	0xB0
#define LCD_FRC				0xB1
#define LCD_BPC				0xB5
#define LCD_DFC				0xB6
#define LCD_POWER1			0xC0
#define LCD_POWER2			0xC1
#define LCD_VCOM1			0xC5
#define LCD_VCOM2			0xC7
#define LCD_POWERA			0xCB
#define LCD_POWERB			0xCF
#define LCD_PGAMMA			0xE0
#define LCD_NGAMMA			0xE1
#define LCD_DTCA			0xE8
#define LCD_DTCB			0xEA
#define LCD_POWER_SEQ		0xED
#define LCD_3GAMMA_EN		0xF2
#define LCD_INTERFACE		0xF6
#define LCD_PRC				0xF7

//Frequently Used Colors
#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

#define SCREEN_VERTICAL_1			0
#define SCREEN_HORIZONTAL_1		1
#define SCREEN_VERTICAL_2			2
#define SCREEN_HORIZONTAL_2		3

#endif /* INC_LCD_DRV_H_ */
