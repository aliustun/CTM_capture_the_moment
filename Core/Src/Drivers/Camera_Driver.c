/*
 * Camera_Driver.c
 *
 *      Written for ELE529E lecture project
 *
 */


#include "Drivers/OV7670.h"
#include "Drivers/OV7670_regs.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1; // CubeMX'te I2C1 kullanıldıysa

static void ov7670_write(uint8_t reg, uint8_t data) {
    HAL_I2C_Mem_Write(&hi2c1, 0x42, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
}

static uint8_t ov7670_read(uint8_t reg) {
    uint8_t val = 0;
    HAL_I2C_Mem_Write(&hi2c1, 0x42, reg, I2C_MEMADD_SIZE_8BIT, NULL, 0, HAL_MAX_DELAY);
    HAL_I2C_Mem_Read(&hi2c1, 0x43, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    return val;
}

int OV7670_Init(void) {
    if (ov7670_read(REG_PID) != 0x76) {
        return -1;
    }

    ov7670_write(REG_COM7, COM7_RESET);
    HAL_Delay(100);

    ov7670_write(REG_CLKRC, 0x01);
    ov7670_write(REG_COM7, COM7_FMT_VGA | COM7_YUV);

    // HREF, VREF vs. burada devam ettirilebilir...
    // sadece temel başlatma burada örneklendi

    return 0;
}
