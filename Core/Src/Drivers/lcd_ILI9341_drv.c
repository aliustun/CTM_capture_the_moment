#include "stm32f4xx_hal.h"



void LCD_ILI9341_Init() {

	LCD_SPI_Init();
	MX_SPI5_Init();

};

void LCD_SPI_Init() {

	SPI_HandleTypeDef hspi;


};
