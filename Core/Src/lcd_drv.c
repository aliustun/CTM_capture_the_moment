/*
 * lcd_drv.c
 *
 *  Created on: May 23, 2025
 *      Author: OVatanse
 */

//SPI PINS
//
//PF7 - SCK
//PF8 - MISO
//PF9 - MOSI

//LCD PINS
//
//PC2	- CSX
//PD13	- DCX

#include "lcd_drv.h"

extern SPI_HandleTypeDef hspi5;

static te_LCD_ERROR_CODES LCD_GPIO_Init(void);
static te_LCD_ERROR_CODES LCD_SPI_Init(void);
static te_LCD_ERROR_CODES LCD_Init(void);


static void LCD_SPI_Send(unsigned char data);
static void LCD_Write_Command(uint8_t command);
static void LCD_Write_Data(uint8_t data);
static void LCD_Set_Cursor_Position(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
static void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color);
static void LCD_Set_Rotation(uint8_t rotation);
static void LCD_Delay(int delay);
static void LCD_Fill_Screen(uint16_t color);
static void LCD_Display_Image(uint16_t image[LCD_WIDTH*LCD_HEIGHT]);


te_LCD_ERROR_CODES LCD_Open(void* vpParam) {
	te_LCD_ERROR_CODES error = E_LCD_ERR_NONE;
	LCD_GPIO_Init();
	error = LCD_SPI_Init();
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	LCD_Init();
	return error;
}

te_LCD_ERROR_CODES LCD_Ioctl(te_LCD_IOCTL_COMMANDS eCommand, void * vpParam) {


	ts_LCD_PIXEL_ATTR pixel_attr; //pixel attribute
	uint16_t color;
	uint16_t * img;
	uint8_t rotation;
	switch (eCommand) {
	case E_LCD_IOCTL_GET_VERSION:
		*(float*)vpParam = LCD_DRIVER_SW_VERSION;
		break;
	case E_LCD_IOCTL_GET_SCREEN_WIDTH:
		*(float*)vpParam = LCD_WIDTH;
		break;
	case E_LCD_IOCTL_GET_SCREEN_HEIGHT:
		*(float*)vpParam = LCD_HEIGHT;
		break;
	case E_LCD_IOCTL_DRAW_PIXEL:
		pixel_attr = *(ts_LCD_PIXEL_ATTR *) vpParam;
		LCD_Draw_Pixel(pixel_attr.x, pixel_attr.y, pixel_attr.color);
		break;
	case E_LCD_IOCTL_FILL_SCREEN:
		color = *(uint16_t*) vpParam;
		LCD_Fill_Screen(color);
		break;
	case E_LCD_IOCTL_DRAW_IMAGE:
		img = (uint16_t *)vpParam;
		LCD_Display_Image(img);
		break;
	case E_LCD_IOCTL_SET_ROTATION:
		rotation = *(uint8_t*)vpParam;
		LCD_Set_Rotation(rotation);
		break;
	default:
		break;
	}
	return E_LCD_ERR_NONE;
};

te_LCD_ERROR_CODES LCD_Write(const void *pvBuffer, const uint32_t xBytes) {
	ts_LCD_WR_TYPE wr_data;

	wr_data = *(ts_LCD_WR_TYPE*) pvBuffer;

	uint32_t n, i, j;
	uint32_t size;

	size = (wr_data.x2 - wr_data.x1) * (wr_data.y2 - wr_data.y1);
	LCD_Set_Cursor_Position(wr_data.x1, wr_data.x2, wr_data.y1, wr_data.y2);

	LCD_Write_Command(LCD_GRAM);

	HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

	for (n = 0; n < size; n++) {
		i = wr_data.img[n] >> 8;
		j = wr_data.img[n] & 0xFF;

		LCD_SPI_Send(i);
		LCD_SPI_Send(j);
	}

	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	return E_LCD_ERR_NONE;
}

te_LCD_ERROR_CODES LCD_Close(void* vpParam) {


	LCD_Write_Command(0x28); // Display OFF
	LCD_Write_Command(0x10); // Enter Sleep Mode
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	HAL_SPI_DeInit(&hspi5);

	return E_LCD_ERR_NONE;
}

static te_LCD_ERROR_CODES LCD_GPIO_Init(void) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_SPI5_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();



	//PC2 - PD13 LCD Pin Configuration
	GPIO_InitStruct.Pin = LCD_CS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_CS_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_WR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_WR_PORT, &GPIO_InitStruct);

	// PF7 - PF8 - PF9 SPI Pin Configuration
	GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	return E_LCD_ERR_NONE;
}

static te_LCD_ERROR_CODES LCD_SPI_Init(void) {
	//SPI Configuration
	hspi5.Instance = SPI5;
	hspi5.Init.Mode = SPI_MODE_MASTER;
	hspi5.Init.Direction = SPI_DIRECTION_2LINES;
	hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi5.Init.NSS = SPI_NSS_SOFT;
	hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi5.Init.CRCPolynomial = 10;

	if (HAL_SPI_Init(&hspi5) != HAL_OK) {
		return E_LCD_ERR_SPI_INIT;
	}

	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	return E_LCD_ERR_NONE;
}

void LCD_Reset(void)
{
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
HAL_Delay(200);
HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
HAL_Delay(200);
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

void LCD_Enable(void)
{
HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

static te_LCD_ERROR_CODES LCD_Init(void) {
		HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);

		LCD_Write_Command(0x01);
		HAL_Delay(1000);

		LCD_Write_Command(LCD_POWERA);
		LCD_Write_Data(0x39);
		LCD_Write_Data(0x2C);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x34);
		LCD_Write_Data(0x02);
		LCD_Write_Command(LCD_POWERB);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0xC1);
		LCD_Write_Data(0x30);
		LCD_Write_Command(LCD_DTCA);
		LCD_Write_Data(0x85);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x78);
		LCD_Write_Command(LCD_DTCB);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x00);
		LCD_Write_Command(LCD_POWER_SEQ);
		LCD_Write_Data(0x64);
		LCD_Write_Data(0x03);
		LCD_Write_Data(0x12);
		LCD_Write_Data(0x81);
		LCD_Write_Command(LCD_PRC);
		LCD_Write_Data(0x20);
		LCD_Write_Command(LCD_POWER1);
		LCD_Write_Data(0x23);
		LCD_Write_Command(LCD_POWER2);
		LCD_Write_Data(0x10);
		LCD_Write_Command(LCD_VCOM1);
		LCD_Write_Data(0x3E);
		LCD_Write_Data(0x28);
		LCD_Write_Command(LCD_VCOM2);
		LCD_Write_Data(0x86);
		LCD_Write_Command(LCD_MAC);
		LCD_Write_Data(0x48);
		LCD_Write_Command(LCD_PIXEL_FORMAT);
		LCD_Write_Data(0x55);
		LCD_Write_Command(LCD_FRC);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x18);
		LCD_Write_Command(LCD_DFC);
		LCD_Write_Data(0x08);
		LCD_Write_Data(0x82);
		LCD_Write_Data(0x27);
		LCD_Write_Command(LCD_3GAMMA_EN);
		LCD_Write_Data(0x00);
		LCD_Write_Command(LCD_COLUMN_ADDR);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0xEF);
		LCD_Write_Command(LCD_PAGE_ADDR);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x01);
		LCD_Write_Data(0x3F);
		LCD_Write_Command(LCD_GAMMA);
		LCD_Write_Data(0x01);
		LCD_Write_Command(LCD_PGAMMA);
		LCD_Write_Data(0x0F);
		LCD_Write_Data(0x31);
		LCD_Write_Data(0x2B);
		LCD_Write_Data(0x0C);
		LCD_Write_Data(0x0E);
		LCD_Write_Data(0x08);
		LCD_Write_Data(0x4E);
		LCD_Write_Data(0xF1);
		LCD_Write_Data(0x37);
		LCD_Write_Data(0x07);
		LCD_Write_Data(0x10);
		LCD_Write_Data(0x03);
		LCD_Write_Data(0x0E);
		LCD_Write_Data(0x09);
		LCD_Write_Data(0x00);
		LCD_Write_Command(LCD_NGAMMA);
		LCD_Write_Data(0x00);
		LCD_Write_Data(0x0E);
		LCD_Write_Data(0x14);
		LCD_Write_Data(0x03);
		LCD_Write_Data(0x11);
		LCD_Write_Data(0x07);
		LCD_Write_Data(0x31);
		LCD_Write_Data(0xC1);
		LCD_Write_Data(0x48);
		LCD_Write_Data(0x08);
		LCD_Write_Data(0x0F);
		LCD_Write_Data(0x0C);
		LCD_Write_Data(0x31);
		LCD_Write_Data(0x36);
		LCD_Write_Data(0x0F);
		LCD_Write_Command(LCD_SLEEP_OUT);

		LCD_Delay(1000000);

		LCD_Write_Command(LCD_DISPLAY_ON);
		LCD_Write_Command(LCD_GRAM);

	//STARTING ROTATION
	LCD_Set_Rotation(SCREEN_HORIZONTAL_2);
	//LCD_Write_Command(LCD_GRAM);

	return E_LCD_ERR_NONE;
}

static void LCD_SPI_Send(unsigned char data) {
	HAL_SPI_Transmit(&hspi5, &data, 1, 1);
}

static void LCD_Write_Command(uint8_t command) {
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_RESET);

	LCD_SPI_Send(command);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_Write_Data(uint8_t data) {
	HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	LCD_SPI_Send(data);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_Set_Cursor_Position(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) {
	LCD_Write_Command(0x2A);
	LCD_Write_Data(x1 >> 8);
	LCD_Write_Data(x1 &  0xFF);
	LCD_Write_Data(x2 >> 8);
	LCD_Write_Data(x2 &  0xFF);

	LCD_Write_Command(0x2B);
	LCD_Write_Data(y1 >> 8);
	LCD_Write_Data(y1 &  0xFF);
	LCD_Write_Data(y2 >> 8);
	LCD_Write_Data(y2 &  0xFF);
}

static void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color) {
	LCD_Set_Cursor_Position(x, x, y, y);
	LCD_Write_Command(LCD_GRAM);
	LCD_Write_Data(color >> 8);
	LCD_Write_Data(color & 0xFF);
}

static void LCD_Fill_Screen(uint16_t color) {
	uint8_t color_msb, color_lsb;

	color_msb = color >> 8;
	color_lsb = color & 0xFF;
	LCD_Set_Cursor_Position(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);
	LCD_Write_Command(LCD_GRAM);

	HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

	for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
		LCD_SPI_Send(color_msb);
		LCD_SPI_Send(color_lsb);
	}

	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void LCD_Delay(int delay) {
	while(delay>0) {
		delay--;
	}
}

static void LCD_Display_Image(uint16_t image[LCD_WIDTH*LCD_HEIGHT]) {
	uint32_t n, i, j;
	LCD_Set_Cursor_Position(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1);

	LCD_Write_Command(LCD_GRAM);

	HAL_GPIO_WritePin(LCD_WR_PORT, LCD_WR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

	for (n = 0; n < LCD_WIDTH * LCD_HEIGHT; n++) {
		i = image[n] >> 8;
		j = image[n] & 0xFF;

		LCD_SPI_Send(i);
		LCD_SPI_Send(j);
	}

	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}



static void LCD_Set_Rotation(uint8_t rotation) {

uint8_t screen_rotation = rotation;

LCD_Write_Command(0x36);
HAL_Delay(1);

switch(screen_rotation)
	{
		case SCREEN_VERTICAL_1:
			LCD_Write_Data(0x40|0x08);
			break;
		case SCREEN_HORIZONTAL_1:
			LCD_Write_Data(0x20|0x08);
			break;
		case SCREEN_VERTICAL_2:
			LCD_Write_Data(0x80|0x08);
			break;
		case SCREEN_HORIZONTAL_2:
			LCD_Write_Data(0x40|0x80|0x20|0x08);
			break;
		default:
			//EXIT IF SCREEN ROTATION NOT VALID!
			break;
	}
}










