/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    memorymap.h
  * @brief   This file contains all the function prototypes for
  *          the memorymap.c file
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
#ifndef __MEMORYMAP_H__
#define __MEMORYMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
void Debug_SetStage(uint8_t stage, const char *message);
void Debug_LogUart5(const char *message);
void Debug_SpinDelay(volatile uint32_t ticks);
void Panel_ClearFramebuffers(uint8_t red, uint8_t green, uint8_t blue);
void Panel_ExitStandby(void);
void DisplayDiag_OnLtdcFifoUnderrun(void);
void DisplayDiag_LogRuntime(void);
void DisplayDiag_OnVSync(void);
void DisplayDiag_OnSwap(void);
void DisplayDiag_OnFrontPorch(void);
void DisplayDiag_OnUiTick(void);
void DisplayDiag_OnUiTelemetryPush(void);
uint32_t DisplayDiag_GetCanRxIsrCount(void);
uint32_t DisplayDiag_GetCanRxDroppedFrames(void);
uint8_t DisplayDiag_GetTelemetryTaskEntered(void);
uint8_t DisplayDiag_GetDisplayCanReady(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __MEMORYMAP_H__ */

