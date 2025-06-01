/*
 * camera_drv.c
 *
 *  Created on: May 24, 2025
 *      Author: OVatanse
 */

//Camera Pins
//PB8 - SIOC
//PB9 - SIOD
//PB7 - VSYNC
//PA4 - HREF
//PA6 - PCLK
//PA8 - XCLK
//PE6 - D7
//PE5 - D6
//PB6 - D5
//PE4 - D4
//PC9 - D3
//PC8 - D2
//PC7 - D1
//PC6 - D0

//RESET - 3v3
//PWDN 	- GND

#include "camera_drv.h"

extern I2C_HandleTypeDef hi2c1;
extern DCMI_HandleTypeDef hdcmi;

static te_CAMERA_ERROR_CODES Camera_Init(void);
void Camera_Delay (volatile uint16_t nCount);
bool Camera_Write(uint8_t reg_addr, uint8_t* data);
static void Camera_GPIO_Init(void);
static te_CAMERA_ERROR_CODES Camera_DCMI_Init(void);
static void Camera_XCLK_Init();
static te_CAMERA_ERROR_CODES Camera_DMA_Init(void);
static te_CAMERA_ERROR_CODES Camera_I2C_Init(void);

extern DMA_HandleTypeDef hdma_dcmi;

const uint8_t OV7670_reg [OV7670_REG_NUM][2] = {
	{0x12, 0x80},		//Reset registers

	// Image format
	{0x12, 0x14},		//QVGA size, RGB mode
	{0x40, 0xd0},		//RGB565
	{0xb0, 0x84},		//Color mode

	// Hardware window
	{0x11, 0x01},		//PCLK settings, 15fps
	{0x32, 0x80},		//HREF
	{0x17, 0x17},		//HSTART
	{0x18, 0x05},		//HSTOP
	{0x03, 0x0a},		//VREF
	{0x19, 0x02},		//VSTART
	{0x1a, 0x7a},		//VSTOP

	// Scalling numbers
	{0x70, 0x3a},		//X_SCALING
	{0x71, 0x35},		//Y_SCALING
	{0x72, 0x11},		//DCW_SCALING
	{0x73, 0xf0},		//PCLK_DIV_SCALING
	{0xa2, 0x02},		//PCLK_DELAY_SCALING

	// Matrix coefficients
	{0x4f, 0x80},		{0x50, 0x80},
	{0x51, 0x00},		{0x52, 0x22},
	{0x53, 0x5e},		{0x54, 0x80},
	{0x58, 0x9e},

	// Gamma curve values
	{0x7a, 0x20},		{0x7b, 0x10},
	{0x7c, 0x1e},		{0x7d, 0x35},
	{0x7e, 0x5a},		{0x7f, 0x69},
	{0x80, 0x76},		{0x81, 0x80},
	{0x82, 0x88},		{0x83, 0x8f},
	{0x84, 0x96},		{0x85, 0xa3},
	{0x86, 0xaf},		{0x87, 0xc4},
	{0x88, 0xd7},		{0x89, 0xe8},

	// AGC and AEC parameters
	{0xa5, 0x05},		{0xab, 0x07},
	{0x24, 0x95},		{0x25, 0x33},
	{0x26, 0xe3},		{0x9f, 0x78},
	{0xa0, 0x68},		{0xa1, 0x03},
	{0xa6, 0xd8},		{0xa7, 0xd8},
	{0xa8, 0xf0},		{0xa9, 0x90},
	{0xaa, 0x94},		{0x10, 0x00},

	// AWB parameters
	{0x43, 0x0a},		{0x44, 0xf0},
	{0x45, 0x34},		{0x46, 0x58},
	{0x47, 0x28},		{0x48, 0x3a},
	{0x59, 0x88},		{0x5a, 0x88},
	{0x5b, 0x44},		{0x5c, 0x67},
	{0x5d, 0x49},		{0x5e, 0x0e},
	{0x6c, 0x0a},		{0x6d, 0x55},
	{0x6e, 0x11},		{0x6f, 0x9f},
	{0x6a, 0x40},		{0x01, 0x40},
	{0x02, 0x60},		{0x13, 0xe7},

	// Additional parameters
	{0x34, 0x11},		{0x3f, 0x00},
	{0x75, 0x05},		{0x76, 0xe1},
	{0x4c, 0x00},		{0x77, 0x01},
	{0xb8, 0x0a},		{0x41, 0x18},
	{0x3b, 0x12},		{0xa4, 0x88},
	{0x96, 0x00},		{0x97, 0x30},
	{0x98, 0x20},		{0x99, 0x30},
	{0x9a, 0x84},		{0x9b, 0x29},
	{0x9c, 0x03},		{0x9d, 0x4c},
	{0x9e, 0x3f},		{0x78, 0x04},
	{0x0e, 0x61},		{0x0f, 0x4b},
	{0x16, 0x02},		{0x1e, 0x00},
	{0x21, 0x02},		{0x22, 0x91},
	{0x29, 0x07},		{0x33, 0x0b},
	{0x35, 0x0b},		{0x37, 0x1d},
	{0x38, 0x71},		{0x39, 0x2a},
	{0x3c, 0x78},		{0x4d, 0x40},
	{0x4e, 0x20},		{0x69, 0x00},
	{0x6b, 0x3a},		{0x74, 0x10},
	{0x8d, 0x4f},		{0x8e, 0x00},
	{0x8f, 0x00},		{0x90, 0x00},
	{0x91, 0x00},		{0x96, 0x00},
	{0x9a, 0x00},		{0xb1, 0x0c},
	{0xb2, 0x0e},		{0xb3, 0x82},
	{0x4b, 0x01},
};

te_CAMERA_ERROR_CODES Camera_Open(void) {
	te_CAMERA_ERROR_CODES error;
	// Camera_GPIO_Init();
	if (error = Camera_I2C_Init() != E_CAMERA_ERR_NONE) return error;
	if (error = Camera_DCMI_Init() != E_CAMERA_ERR_NONE) return error;
	if (error = Camera_DMA_Init() != E_CAMERA_ERR_NONE) return error;
	Camera_XCLK_Init();
	if (error = Camera_Init() != E_CAMERA_ERR_NONE) return error;
	return E_CAMERA_ERR_NONE;
}

void Camera_Delay (volatile uint16_t nCount){
  while(nCount--){
  }
}

static te_CAMERA_ERROR_CODES Camera_Init(void) {
	uint8_t data, i = 0;
	bool err;
	LCD_Fill_Screen(PURPLE);
	LCD_Set_Rotation(SCREEN_HORIZONTAL_1);
	// Configure camera registers
	for(i=0; i<OV7670_REG_NUM ;i++){
		data = OV7670_reg[i][1];
		err = Camera_Write(OV7670_reg[i][0], &data);

		if (err == true)
			return E_CAMERA_ERR_CAMERA_INIT;
		Camera_Delay(0xFFFF);
	}

	return E_CAMERA_ERR_NONE;
}

bool Camera_Write(uint8_t reg_addr, uint8_t* data)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write(
        &hi2c1,
        OV7670_WRITE_ADDR,
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        data,
        1,
        HAL_MAX_DELAY
    );

    return (status != HAL_OK);
}

static void Camera_GPIO_Init(void) {
	__HAL_RCC_DCMI_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_I2C1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();


	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//PA8 (XCLK)
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PA4 (HREF), PA6 (PCLK)
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PB6 (D5), PB7 (VSYNC)
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// PC6 (D0), PC7 (D1), PC8 (D2), PC9 (D3)
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	// PE4 (D4), PE5 (D6), PE6 (D7)
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;         // Open-drain mode (I2C için zorunlu)
	GPIO_InitStruct.Pull = GPIO_PULLUP;             // Pull-up aktif
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    // İsteğe göre değişebilir (LOW/VERY_HIGH)
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

static te_CAMERA_ERROR_CODES Camera_DCMI_Init(void) {
	hdcmi.Instance = DCMI;
	hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
	hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
	hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
	hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;

	if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
		return E_CAMERA_ERR_DCMI_INIT;
	}

	return E_CAMERA_ERR_NONE;
}

static void Camera_XCLK_Init() {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_RCC_EnableCSS();
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);
}

static te_CAMERA_ERROR_CODES Camera_DMA_Init(void) {
	 /* DMA controller clock enable */
	  __HAL_RCC_DMA2_CLK_ENABLE();

	hdma_dcmi.Instance = DMA2_Stream1;
	hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
	hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
	hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_dcmi.Init.Mode = DMA_CIRCULAR;
	hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma_dcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_dcmi.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_dcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;

	__HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi);
	if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
		return E_CAMERA_ERR_DMA_INIT;
	}

	// DMA interrupt'ı enable et
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	return E_CAMERA_ERR_NONE;
}

static te_CAMERA_ERROR_CODES Camera_I2C_Init(void) {
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
	return E_CAMERA_ERR_I2C_INIT;
	}

	/** Configure Analogue filter
	*/
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
	  return E_CAMERA_ERR_I2C_INIT;
	}

	/** Configure Digital filter
	*/
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
	  return E_CAMERA_ERR_I2C_INIT;
	}

	return E_CAMERA_ERR_NONE;
}


