/*
 * OV7670.h
 *
 *      Written for ELE529E lecture project
 *
 */

#ifndef __OV7670_H
#define __OV7670_H

#include "stm32f4xx.h"  // STM32F4xx CMSIS başlık dosyası

// ----------------------------
// Kamera Görüntü Boyutları
// ----------------------------
#define OV7670_RES_X        176   // Genişlik
#define OV7670_RES_Y        144   // Yükseklik

// Frame buffer (YUV422 → her piksel için 2 byte olabilir)
__IO uint16_t RAM_Buffer[OV7670_RES_X * OV7670_RES_Y];

// ----------------------------
// DMA Ayarları (Kamera için)
// ----------------------------

// DMA akışı ve kanalı
#define CAMERA_DMA_STREAM               DMA2_Stream7
#define CAMERA_DMA_CHANNEL              DMA_Channel_1
#define CAMERA_DMA_STREAM_CLK           RCC_AHB1Periph_DMA2

// DMA kesme ayarları
#define CAMERA_DMA_IRQ                  DMA2_Stream7_IRQn
#define CAMERA_DMA_IRQHandler           DMA2_Stream7_IRQHandler
#define CAMERA_DMA_IT_TCIF              DMA_IT_TCIF7

// DCMI veri register adresi (sabit)
#define DCMI_DR_ADDRESS                 0x50050028  // DCMI->DR

// ----------------------------
// OV7670 Kamera Fonksiyonları
// ----------------------------

/**
 * @brief  OV7670 register'dan veri okuma
 * @param  reg: register adresi
 * @retval okunan veri
 */
uint8_t ov7670_get(uint8_t reg);

/**
 * @brief  OV7670 register'a veri yazma
 * @param  reg: register adresi
 * @param  data: yazılacak veri
 * @retval işlem sonucu
 */
uint8_t ov7670_set(uint8_t reg, uint8_t data);

/**
 * @brief  OV7670 başlatma (I2C üzerinden konfigürasyon)
 * @retval 0: başarı, 1: hata
 */
int ov7670_init(void);

/**
 * @brief  DCMI periferik başlatma
 */
void DCMI_init(void);

/**
 * @brief  DMA başlatma (DCMI için)
 */
void DMA_init(void);

#endif // __OV7670_H
