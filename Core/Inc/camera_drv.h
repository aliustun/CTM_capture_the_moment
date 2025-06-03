/*
 * camera_drv.h
 *
 *  Created on: May 24, 2025
 *      Author: OVatanse
 */

#ifndef INC_CAMERA_DRV_H_
#define INC_CAMERA_DRV_H_

#include "stm32f4xx.h"
#include "lcd_drv.h"
#include <stdbool.h>

typedef enum {
	E_CAMERA_ERR_NONE,
	E_CAMERA_ERR_I2C_INIT,
	E_CAMERA_ERR_DCMI_INIT,
	E_CAMERA_ERR_DMA_INIT,
	E_CAMERA_ERR_CAMERA_INIT,
} te_CAMERA_ERROR_CODES;

te_CAMERA_ERROR_CODES Camera_Open(void);

#define SCCB_REG_ADDR 			0x01

// OV7670 camera settings
#define OV7670_REG_NUM 			122
#define OV7670_WRITE_ADDR 	0x42

// Image settings
#define IMG_ROWS   			    160
#define IMG_COLUMNS   			240



#endif /* INC_CAMERA_DRV_H_ */
