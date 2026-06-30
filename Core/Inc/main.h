/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Debug_LogUart5(const char *message);
void Debug_SetRuntimeLogsEnabled(uint8_t enabled);
extern volatile uint8_t g_ltdc_stage;
extern volatile uint32_t g_ltdc_fifo_underrun_count;
extern volatile uint32_t g_ltdc_fifo_underrun_last_isr;
extern volatile uint32_t g_ltdc_fifo_underrun_last_cpsr;
extern volatile uint8_t g_sd_preos_done;
extern volatile uint8_t g_sd_preos_ok;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_Pin GPIO_PIN_2
#define SPI_CS_GPIO_Port GPIOE
#define SPI_CE_Pin GPIO_PIN_3
#define SPI_CE_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOI
#define LED3_Pin GPIO_PIN_13
#define LED3_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOA
#define LTDC_STDBY_Pin GPIO_PIN_5
#define LTDC_STDBY_GPIO_Port GPIOC
#define DOWN_SW_Pin GPIO_PIN_6
#define DOWN_SW_GPIO_Port GPIOH
#define SELECT_SW_Pin GPIO_PIN_7
#define SELECT_SW_GPIO_Port GPIOH
#define UP_SW_Pin GPIO_PIN_8
#define UP_SW_GPIO_Port GPIOH
#define MENU_SW_Pin GPIO_PIN_9
#define MENU_SW_GPIO_Port GPIOH
#define MICROSD_DET_Pin GPIO_PIN_7
#define MICROSD_DET_GPIO_Port GPIOC
#define SPI_IRQ_Pin GPIO_PIN_5
#define SPI_IRQ_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
