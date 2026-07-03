/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "fdcan.h"
#include "memorymap.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_TELEMETRY_PERIOD_MS     200U
#define CAN_TELEMETRY_OK_PULSE_MS   5U
#define CAN_TELEMETRY_ERR_PULSE_MS  20U
#define CAN_TELEMETRY_HANDLE        hfdcan1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static FDCAN_TxHeaderTypeDef can_tx_header;
static uint16_t can_telemetry_sequence = 0U;
static volatile uint8_t g_dbg_stage = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
static void CAN_Telemetry_Init(void);
static HAL_StatusTypeDef CAN_Telemetry_SendFrame(uint32_t id, const uint8_t* data, uint32_t len);
static HAL_StatusTypeDef CAN_Telemetry_SendAll(void);
static uint8_t CAN_Telemetry_HasBusError(void);
static void CAN_Telemetry_PulseOkStatus(void);
static void CAN_Telemetry_PulseErrStatus(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void DBG_Log(const char *msg)
{
  (void)HAL_UART_Transmit(&huart10, (const uint8_t *)msg, (uint16_t)strlen(msg), 200U);
}

static void DBG_LogFmt(const char *fmt, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
  char buf[96];
  (void)snprintf(buf, sizeof(buf), fmt,
                 (unsigned long)a, (unsigned long)b,
                 (unsigned long)c, (unsigned long)d);
  DBG_Log(buf);
}

static uint32_t CAN_Telemetry_DlcFromLen(uint32_t len)
{
  switch (len)
  {
    case 2U: return FDCAN_DLC_BYTES_2;
    case 4U: return FDCAN_DLC_BYTES_4;
    case 6U: return FDCAN_DLC_BYTES_6;
    case 7U: return FDCAN_DLC_BYTES_7;
    case 8U: return FDCAN_DLC_BYTES_8;
    default: return FDCAN_DLC_BYTES_0;
  }
}

static void CAN_Telemetry_PutU16(uint8_t* data, uint32_t offset, uint16_t value)
{
  data[offset] = (uint8_t)(value & 0xFFU);
  data[offset + 1U] = (uint8_t)((value >> 8) & 0xFFU);
}

static void CAN_Telemetry_PutS16(uint8_t* data, uint32_t offset, int16_t value)
{
  CAN_Telemetry_PutU16(data, offset, (uint16_t)value);
}

static void CAN_Telemetry_PutU32(uint8_t* data, uint32_t offset, uint32_t value)
{
  data[offset] = (uint8_t)(value & 0xFFU);
  data[offset + 1U] = (uint8_t)((value >> 8) & 0xFFU);
  data[offset + 2U] = (uint8_t)((value >> 16) & 0xFFU);
  data[offset + 3U] = (uint8_t)((value >> 24) & 0xFFU);
}

static void CAN_Telemetry_PutS32(uint8_t* data, uint32_t offset, int32_t value)
{
  CAN_Telemetry_PutU32(data, offset, (uint32_t)value);
}

static void CAN_Telemetry_Init(void)
{
  can_tx_header.Identifier = 0U;
  can_tx_header.IdType = FDCAN_STANDARD_ID;
  can_tx_header.TxFrameType = FDCAN_DATA_FRAME;
  can_tx_header.DataLength = FDCAN_DLC_BYTES_8;
  can_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  can_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
  can_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
  can_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  can_tx_header.MessageMarker = 0;

  if (HAL_FDCAN_Start(&CAN_TELEMETRY_HANDLE) != HAL_OK)
  {
    Error_Handler();
  }
}

static HAL_StatusTypeDef CAN_Telemetry_SendFrame(uint32_t id, const uint8_t* data, uint32_t len)
{
  can_tx_header.Identifier = id;
  can_tx_header.DataLength = CAN_Telemetry_DlcFromLen(len);

  if (HAL_FDCAN_GetTxFifoFreeLevel(&CAN_TELEMETRY_HANDLE) == 0U)
  {
    return HAL_BUSY;
  }

  return HAL_FDCAN_AddMessageToTxFifoQ(&CAN_TELEMETRY_HANDLE, &can_tx_header, (uint8_t*)data);
}

static HAL_StatusTypeDef CAN_Telemetry_SendAll(void)
{
  const uint16_t seq = can_telemetry_sequence++;
  const uint32_t tick_ms = HAL_GetTick();
  const uint16_t accel1 = (uint16_t)(15U + (seq % 70U));
  const uint16_t accel2 = (uint16_t)(18U + ((seq + 7U) % 65U));
  const uint16_t brake = (uint16_t)((seq / 4U) % 90U);
  const int32_t rpm = (int32_t)(1200 + ((int32_t)(seq % 80U) * 35));
  const int32_t inv_speed = (int32_t)(35 + (seq % 80U));
  const int32_t inv_current = (int32_t)(25 + (seq % 45U));
  const uint8_t sd_enabled = (uint8_t)((seq / 50U) & 1U);
  HAL_StatusTypeDef status;
  uint8_t data[8];

  data[0] = 3U;
  data[1] = (uint8_t)(40U + (seq % 35U));
  data[2] = 0x03U;
  data[3] = 1U;
  data[4] = 1U;
  data[5] = sd_enabled;
  CAN_Telemetry_PutU16(data, 6U, seq);
  status = CAN_Telemetry_SendFrame(0x510U, data, 8U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, accel1);
  CAN_Telemetry_PutU16(data, 2U, accel2);
  CAN_Telemetry_PutU16(data, 4U, brake);
  status = CAN_Telemetry_SendFrame(0x511U, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(360U + (seq % 35U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(3300U + (seq % 120U)));
  data[4] = 0U;
  data[5] = 1U;
  status = CAN_Telemetry_SendFrame(0x512U, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS16(data, 0U, (int16_t)(45 + (seq % 18U)));
  CAN_Telemetry_PutS16(data, 2U, (int16_t)(38 + (seq % 16U)));
  CAN_Telemetry_PutS16(data, 4U, (int16_t)(30 + (seq % 10U)));
  status = CAN_Telemetry_SendFrame(0x513U, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS32(data, 0U, rpm);
  status = CAN_Telemetry_SendFrame(0x514U, data, 4U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS32(data, 0U, inv_speed);
  status = CAN_Telemetry_SendFrame(0x515U, data, 4U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS32(data, 0U, inv_current);
  status = CAN_Telemetry_SendFrame(0x516U, data, 4U);
  if (status != HAL_OK) { return status; }

  data[0] = (uint8_t)(seq % 6U);
  data[1] = (uint8_t)(seq % 5U);
  status = CAN_Telemetry_SendFrame(0x517U, data, 2U);
  if (status != HAL_OK) { return status; }

  data[0] = (uint8_t)(85U - (seq % 12U));
  CAN_Telemetry_PutS16(data, 1U, (int16_t)(18 + (seq % 20U)));
  CAN_Telemetry_PutS16(data, 3U, (int16_t)(4 + (seq % 8U)));
  CAN_Telemetry_PutS16(data, 5U, (int16_t)(34 + (seq % 8U)));
  status = CAN_Telemetry_SendFrame(0x518U, data, 7U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(45U + (seq % 70U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(900U + (seq % 220U)));
  CAN_Telemetry_PutS32(data, 4U, (int32_t)(650 + (seq % 20U)));
  status = CAN_Telemetry_SendFrame(0x519U, data, 8U);
  if (status != HAL_OK) { return status; }

  data[0] = 3U;
  data[1] = (uint8_t)(8U + (seq % 4U));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(80U + (seq % 15U)));
  CAN_Telemetry_PutS32(data, 4U, (int32_t)(407123456 + (int32_t)seq));
  status = CAN_Telemetry_SendFrame(0x51AU, data, 8U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS32(data, 0U, (int32_t)(-37456789 - (int32_t)seq));
  CAN_Telemetry_PutU32(data, 4U, tick_ms);
  status = CAN_Telemetry_SendFrame(0x51BU, data, 8U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(3300U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(3310U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 4U, (uint16_t)(3320U + (seq % 10U)));
  status = CAN_Telemetry_SendFrame(0x51CU, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(3330U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(3340U + (seq % 10U)));
  status = CAN_Telemetry_SendFrame(0x51DU, data, 4U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(4150U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(4160U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 4U, (uint16_t)(4170U + (seq % 10U)));
  status = CAN_Telemetry_SendFrame(0x51EU, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutU16(data, 0U, (uint16_t)(4180U + (seq % 10U)));
  CAN_Telemetry_PutU16(data, 2U, (uint16_t)(4190U + (seq % 10U)));
  status = CAN_Telemetry_SendFrame(0x51FU, data, 4U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS16(data, 0U, (int16_t)(32 + (seq % 8U)));
  CAN_Telemetry_PutS16(data, 2U, (int16_t)(34 + (seq % 8U)));
  CAN_Telemetry_PutS16(data, 4U, (int16_t)(36 + (seq % 8U)));
  status = CAN_Telemetry_SendFrame(0x520U, data, 6U);
  if (status != HAL_OK) { return status; }

  CAN_Telemetry_PutS16(data, 0U, (int16_t)(38 + (seq % 8U)));
  CAN_Telemetry_PutS16(data, 2U, (int16_t)(40 + (seq % 8U)));
  CAN_Telemetry_PutS16(data, 4U, (int16_t)(34 + (seq % 8U)));
  return CAN_Telemetry_SendFrame(0x521U, data, 6U);
}

static uint8_t CAN_Telemetry_HasBusError(void)
{
  FDCAN_ProtocolStatusTypeDef protocolStatus = {0};
  FDCAN_ErrorCountersTypeDef errorCounters = {0};

  if ((HAL_FDCAN_GetProtocolStatus(&CAN_TELEMETRY_HANDLE, &protocolStatus) != HAL_OK) ||
      (HAL_FDCAN_GetErrorCounters(&CAN_TELEMETRY_HANDLE, &errorCounters) != HAL_OK))
  {
    return 1U;
  }

  return ((protocolStatus.BusOff != 0U) ||
          (protocolStatus.ErrorPassive != 0U) ||
          (errorCounters.TxErrorCnt != 0U)) ? 1U : 0U;
}

static void CAN_Telemetry_PulseOkStatus(void)
{
  HAL_GPIO_WritePin(OK_STATUS_GPIO_Port, OK_STATUS_Pin, GPIO_PIN_SET);
  HAL_Delay(CAN_TELEMETRY_OK_PULSE_MS);
  HAL_GPIO_WritePin(OK_STATUS_GPIO_Port, OK_STATUS_Pin, GPIO_PIN_RESET);
}

static void CAN_Telemetry_PulseErrStatus(void)
{
  HAL_GPIO_WritePin(ERR_STATUS_GPIO_Port, ERR_STATUS_Pin, GPIO_PIN_SET);
  HAL_Delay(CAN_TELEMETRY_ERR_PULSE_MS);
  HAL_GPIO_WritePin(ERR_STATUS_GPIO_Port, ERR_STATUS_Pin, GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  MX_USART10_UART_Init();
  DBG_Log("[BOOT] HAL init ok\r\n");
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  DBG_Log("[BOOT] sysclk ok\r\n");

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  MX_USART10_UART_Init();   /* reinit BRR with correct APB2 clock */
  DBG_Log("[BOOT] clocks ok\r\n");
  g_dbg_stage = 1U;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  DBG_Log("[BOOT] gpio ok\r\n");
  g_dbg_stage = 2U;

  MX_FDCAN1_Init();
  DBG_Log("[BOOT] fdcan1 init ok\r\n");
  g_dbg_stage = 3U;

  /* USER CODE BEGIN 2 */
  CAN_Telemetry_Init();
  DBG_Log("[BOOT] can start ok - entering loop\r\n");
  g_dbg_stage = 4U;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_StatusTypeDef can_status;
    uint32_t pulse_time_ms = 0U;

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    can_status = CAN_Telemetry_SendAll();

    if ((can_status == HAL_OK) && (CAN_Telemetry_HasBusError() == 0U))
    {
      CAN_Telemetry_PulseOkStatus();
      pulse_time_ms = CAN_TELEMETRY_OK_PULSE_MS;
    }
    else
    {
      FDCAN_ProtocolStatusTypeDef psr = {0};
      CAN_Telemetry_PulseErrStatus();
      pulse_time_ms = CAN_TELEMETRY_ERR_PULSE_MS;

      if ((HAL_FDCAN_GetProtocolStatus(&CAN_TELEMETRY_HANDLE, &psr) == HAL_OK) &&
          (psr.BusOff != 0U))
      {
        DBG_Log("[CAN] bus-off: restarting\r\n");
        (void)HAL_FDCAN_Stop(&CAN_TELEMETRY_HANDLE);
        HAL_Delay(10U);
        (void)HAL_FDCAN_Start(&CAN_TELEMETRY_HANDLE);
      }
    }

    if (CAN_TELEMETRY_PERIOD_MS > pulse_time_ms)
    {
      HAL_Delay(CAN_TELEMETRY_PERIOD_MS - pulse_time_ms);
    }

    {
      static uint32_t s_dbg_cnt = 0U;
      if (++s_dbg_cnt >= 10U)
      {
        s_dbg_cnt = 0U;
        uint32_t psr = FDCAN1->PSR;
        uint32_t ecr = FDCAN1->ECR;
        DBG_LogFmt("[CAN] seq=%lu PSR=%08lX LEC=%lu TEC=%lu\r\n",
                   (uint32_t)can_telemetry_sequence,
                   psr,
                   psr & 0x7U,
                   ecr & 0xFFU);
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SDMMC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 16;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 1;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  {
    char buf[48];
    (void)snprintf(buf, sizeof(buf), "[ERR] halt at stage=%u\r\n", (unsigned)g_dbg_stage);
    (void)HAL_UART_Transmit(&huart10, (uint8_t *)buf, (uint16_t)strlen(buf), 500U);
  }
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
