/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    memorymap.c
  * @brief   This file provides code for the configuration
  *          of the MEMORYMAP instances.
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
/* Includes ------------------------------------------------------------------*/
#include "memorymap.h"
#include "gpio.h"
#include "ltdc.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_gcc.h"
#include <stdio.h>
#include <string.h>
#include "fdcan.h"

/* USER CODE BEGIN 0 */

#define LCD_WIDTH            ((uint32_t)800U)
#define LCD_HEIGHT           ((uint32_t)480U)
#define LCD_BPP_BYTES        ((uint32_t)2U)
#define LCD_FB_SIZE_BYTES    (LCD_WIDTH * LCD_HEIGHT * LCD_BPP_BYTES)
#define LCD_FB_ADDRESS       ((uint32_t)0xC0000000U)
#define LCD_FB_ADDRESS_ALT   (LCD_FB_ADDRESS + LCD_FB_SIZE_BYTES)

volatile uint8_t g_ltdc_stage = 0U;
volatile uint32_t g_ltdc_fifo_underrun_count = 0U;
volatile uint32_t g_ltdc_fifo_underrun_last_isr = 0U;
volatile uint32_t g_ltdc_fifo_underrun_last_cpsr = 0U;
volatile uint32_t g_ltdc_vsync_count = 0U;
volatile uint32_t g_ltdc_swap_count = 0U;
volatile uint32_t g_ltdc_front_porch_count = 0U;
volatile uint32_t g_ui_tick_count = 0U;
volatile uint32_t g_ui_telemetry_push_count = 0U;
static volatile uint8_t g_debug_uart_busy = 0U;
static volatile uint8_t g_debug_runtime_logs_enabled = 1U;

static uint8_t Debug_IsCriticalMessage(const char *message)
{
  if (message == NULL)
  {
    return 0U;
  }

  return ((strncmp(message, "[FAULT]", 7U) == 0) ||
          (strncmp(message, "[RTOS] STACK OVERFLOW", 21U) == 0) ||
          (strncmp(message, "[RTOS] MALLOC FAILED", 20U) == 0)) ? 1U : 0U;
}

static uint8_t Debug_IsAllowedRuntimeMessage(const char *message)
{
  if (message == NULL)
  {
    return 0U;
  }

  return ((strncmp(message, "[RTOS] telemetryTask entered", 28U) == 0) ||
          (strncmp(message, "[RTOS] display telemetry task ready", 35U) == 0) ||
          (strncmp(message, "[RTOS] defaultTask entered", 26U) == 0) ||
          (strncmp(message, "[RTOS] ui buttons ready", 23U) == 0) ||
          (strncmp(message, "[RTOS] scheduler alive", 22U) == 0) ||
          (strncmp(message, "[RTOS] canTxTask entered", 24U) == 0) ||
          (strncmp(message, "[RTOS] sdTask entered", 21U) == 0) ||
          (strncmp(message, "[CAN]", 5U) == 0) ||
          (strncmp(message, "[DISP]", 6U) == 0) ||
          (strncmp(message, "[SD]", 4U) == 0) ||
          (strncmp(message, "[CFG]", 5U) == 0) ||
          (strncmp(message, "[SDHAL]", 7U) == 0)) ? 1U : 0U;
}

static void LTDC_FillBufferColor(uint32_t address, uint8_t red, uint8_t green, uint8_t blue)
{
  uint16_t color = ((uint16_t)(red & 0xF8U) << 8)
                 | ((uint16_t)(green & 0xFCU) << 3)
                 | ((uint16_t)(blue) >> 3);
  uint16_t *fb = (uint16_t *)address;
  uint32_t pixel_count = LCD_WIDTH * LCD_HEIGHT;

  for (uint32_t pixel = 0U; pixel < pixel_count; pixel++)
  {
    fb[pixel] = color;
  }
}

void Debug_SetStage(uint8_t stage, const char *message)
{
  g_ltdc_stage = stage;

  Debug_LogUart5(message);
}

void Debug_SetRuntimeLogsEnabled(uint8_t enabled)
{
  g_debug_runtime_logs_enabled = enabled;
}

void Debug_LogUart5(const char *message)
{
  uint32_t irqNumber;

  if ((message == NULL) || (huart5.gState == HAL_UART_STATE_RESET))
  {
    return;
  }

  irqNumber = __get_IPSR();
  if (irqNumber != 0U)
  {
    return;
  }

  if ((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) &&
      (g_debug_runtime_logs_enabled == 0U) &&
      (Debug_IsCriticalMessage(message) == 0U) &&
      (Debug_IsAllowedRuntimeMessage(message) == 0U))
  {
    return;
  }

  if ((g_debug_uart_busy != 0U) || (huart5.gState != HAL_UART_STATE_READY))
  {
    return;
  }

  g_debug_uart_busy = 1U;
  (void)HAL_UART_Transmit(&huart5, (uint8_t *)message, (uint16_t)strlen(message), 2U);
  g_debug_uart_busy = 0U;
}

void Debug_SpinDelay(volatile uint32_t ticks)
{
  while (ticks-- != 0U)
  {
    __NOP();
  }
}

void Panel_ClearFramebuffers(uint8_t red, uint8_t green, uint8_t blue)
{
  LTDC_FillBufferColor(LCD_FB_ADDRESS, red, green, blue);
  LTDC_FillBufferColor(LCD_FB_ADDRESS_ALT, red, green, blue);

  if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
  {
    SCB_CleanDCache();
  }
}

void Panel_ExitStandby(void)
{
  HAL_GPIO_WritePin(LTDC_STDBY_GPIO_Port, LTDC_STDBY_Pin, GPIO_PIN_SET);
  HAL_Delay(50U);
  Debug_LogUart5("[BOOT] LTDC_STDBY high\r\n");
}

void DisplayDiag_OnLtdcFifoUnderrun(void)
{
  char buffer[160];
  static uint8_t fuLogCount = 0U;

  g_ltdc_fifo_underrun_count++;
  g_ltdc_fifo_underrun_last_isr = LTDC->ISR;
  g_ltdc_fifo_underrun_last_cpsr = LTDC->CPSR;

  if (fuLogCount < 8U)
  {
    fuLogCount++;
    (void)snprintf(buffer,
                   sizeof(buffer),
                   "[LTDC] FIFO underrun cnt=%lu isr=%08lX cpsr=%08lX cfbar=%08lX\r\n",
                   (unsigned long)g_ltdc_fifo_underrun_count,
                   (unsigned long)g_ltdc_fifo_underrun_last_isr,
                   (unsigned long)g_ltdc_fifo_underrun_last_cpsr,
                   (unsigned long)LTDC_Layer1->CFBAR);
    Debug_LogUart5(buffer);
  }
}

void DisplayDiag_LogRuntime(void)
{
  char buffer[224];
  static uint32_t lastFuCount = 0U;
  static uint32_t lastVSyncCount = 0U;
  static uint32_t lastSwapCount = 0U;
  static uint32_t lastFrontPorchCount = 0U;
  static uint32_t lastUiTickCount = 0U;
  static uint32_t lastUiTelemetryPushCount = 0U;
  static uint32_t lastCanRxCount = 0U;
  uint32_t fuCount = g_ltdc_fifo_underrun_count;
  uint32_t vSyncCount = g_ltdc_vsync_count;
  uint32_t swapCount = g_ltdc_swap_count;
  uint32_t frontPorchCount = g_ltdc_front_porch_count;
  uint32_t uiTickCount = g_ui_tick_count;
  uint32_t uiTelemetryPushCount = g_ui_telemetry_push_count;
  uint32_t canRxCount = DisplayDiag_GetCanRxIsrCount();
  uint32_t canDropCount = DisplayDiag_GetCanRxDroppedFrames();

  if (huart5.gState == HAL_UART_STATE_RESET)
  {
    return;
  }

  (void)snprintf(buffer,
                 sizeof(buffer),
                 "[DISP] 1s canRx=%lu canDrop=%lu fu=%lu(+%ld) vs=%lu sw=%lu fp=%lu uiTick=%lu uiPush=%lu cfbar=%08lX cpsr=%08lX cdsr=%08lX\r\n",
                 (unsigned long)(canRxCount - lastCanRxCount),
                 (unsigned long)canDropCount,
                 (unsigned long)fuCount,
                 (long)(fuCount - lastFuCount),
                 (unsigned long)(vSyncCount - lastVSyncCount),
                 (unsigned long)(swapCount - lastSwapCount),
                 (unsigned long)(frontPorchCount - lastFrontPorchCount),
                 (unsigned long)(uiTickCount - lastUiTickCount),
                 (unsigned long)(uiTelemetryPushCount - lastUiTelemetryPushCount),
                 (unsigned long)LTDC_Layer1->CFBAR,
                 (unsigned long)LTDC->CPSR,
                 (unsigned long)LTDC->CDSR);
  Debug_LogUart5(buffer);

  {
    char fdcan_buf[128];
    uint32_t psr1 = FDCAN1->PSR;
    uint32_t ecr1 = FDCAN1->ECR;
    uint32_t psr2 = FDCAN2->PSR;
    uint32_t ecr2 = FDCAN2->ECR;
    (void)snprintf(fdcan_buf, sizeof(fdcan_buf),
                   "[CAN] FD1 LEC=%lu REC=%lu TEC=%lu | FD2 LEC=%lu REC=%lu TEC=%lu\r\n",
                   (unsigned long)(psr1 & 0x7U),
                   (unsigned long)((ecr1 >> 8U) & 0x7FU),
                   (unsigned long)(ecr1 & 0xFFU),
                   (unsigned long)(psr2 & 0x7U),
                   (unsigned long)((ecr2 >> 8U) & 0x7FU),
                   (unsigned long)(ecr2 & 0xFFU));
    Debug_LogUart5(fdcan_buf);
  }

  lastFuCount = fuCount;
  lastVSyncCount = vSyncCount;
  lastSwapCount = swapCount;
  lastFrontPorchCount = frontPorchCount;
  lastUiTickCount = uiTickCount;
  lastUiTelemetryPushCount = uiTelemetryPushCount;
  lastCanRxCount = canRxCount;
}

void DisplayDiag_OnVSync(void)
{
  g_ltdc_vsync_count++;
}

void DisplayDiag_OnSwap(void)
{
  g_ltdc_swap_count++;
}

void DisplayDiag_OnFrontPorch(void)
{
  g_ltdc_front_porch_count++;
}

void DisplayDiag_OnUiTick(void)
{
  g_ui_tick_count++;
}

void DisplayDiag_OnUiTelemetryPush(void)
{
  g_ui_telemetry_push_count++;
}

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
