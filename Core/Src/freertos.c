/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fdcan.h"
#include "fatfs.h"
#include "sdmmc.h"
#include "app_config.h"
#include "display_telemetry.h"
#include "display_telemetry_can_config.h"
#include "memorymap.h"
#include "ui_buttons.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  FDCAN_RxHeaderTypeDef header;
  uint8_t data[8];
  uint8_t length;
  uint32_t tick;
} CanRxFrame;

typedef struct
{
  DisplayTelemetry snapshot;
  uint32_t sequence;
  uint32_t tick;
  uint32_t sourceCanId;
  uint8_t closeRequest;
  uint8_t configSaveRequest;
} TelemetryFanoutPacket;

typedef enum
{
  CONFIG_SOURCE_CAN = 0U,
  CONFIG_SOURCE_LOCAL = 1U
} ConfigSource;

typedef struct
{
  uint8_t source;
  uint8_t key;
  uint16_t value;
  uint32_t originId;
  uint32_t tick;
} ConfigMessage;

typedef struct
{
  FDCAN_TxHeaderTypeDef header;
  uint8_t data[8];
  uint8_t length;
} CanTxMessage;

typedef struct
{
  uint8_t sdLoggingEnabled;
  uint16_t nextLogSession;
} LocalConfigState;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_RX_QUEUE_LENGTH             32U
#define CAN_TX_QUEUE_LENGTH             16U
#define SD_LOG_QUEUE_LENGTH             16U
#define SD_LOG_MIN_PERIOD_MS            200U
#define CAN_CONFIG_RX_ID                0x120U
#define CAN_CONFIG_TX_ID                0x121U
#define CONFIG_FILE_PATH                "0:/CONFIG.CSV"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static volatile DisplayTelemetry g_displayTelemetry = {0};
static volatile uint32_t g_displayTelemetryWriteSequence = 0U;
static volatile uint32_t g_displayTelemetrySnapshotSequence = 0U;
static volatile uint8_t g_displayTelemetryValid = 0U;
static volatile uint32_t g_displayTelemetryPendingMask = 0U;
static uint8_t g_uiButtonsInitialized = 0U;
static volatile uint32_t g_canRxIsrCount = 0U;
static volatile uint32_t g_canRxDroppedFrames = 0U;
static volatile uint8_t g_telemetryTaskEntered = 0U;
static volatile uint8_t g_displayCanReady = 0U;
static volatile uint32_t g_sdLogDroppedFrames = 0U;
static volatile uint32_t g_sdLogQueuedFrames = 0U;
static volatile uint8_t g_sdLogRecordingActive = 0U;
static volatile uint8_t g_sdLogCreateRequested = 0U;
static volatile uint32_t g_canTxDroppedFrames = 0U;
static volatile LocalConfigState g_localConfig = {1U, 0U};
static volatile VehicleConfigParams g_vehicleConfig = {{0U, 0U, 0U, 0U, 0U}};
static uint8_t g_sdFatFsMounted = 0U;
static uint8_t g_configLoadDone = 0U;
static char g_sdLogFilePath[18] = "";
static char g_sdLogSessionId[13] = "NOLOG";
extern uint8_t sdmmc1_init_ok;
static osMessageQueueId_t canRxQueueHandle;
static osMessageQueueId_t canTxQueueHandle;
static osMessageQueueId_t sdLogQueueHandle;
/* USER CODE END Variables */

uint32_t DisplayDiag_GetCanRxIsrCount(void)
{
  return g_canRxIsrCount;
}

uint32_t DisplayDiag_GetCanRxDroppedFrames(void)
{
  return g_canRxDroppedFrames;
}

uint8_t DisplayDiag_GetTelemetryTaskEntered(void)
{
  return g_telemetryTaskEntered;
}

uint8_t DisplayDiag_GetDisplayCanReady(void)
{
  return g_displayCanReady;
}
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for telemetryTask */
osThreadId_t telemetryTaskHandle;
const osThreadAttr_t telemetryTask_attributes = {
  .name = "telemetryTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for canTxTask */
osThreadId_t canTxTaskHandle;
const osThreadAttr_t canTxTask_attributes = {
  .name = "canTxTask",
  .stack_size = 4096 * 2,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for TouchGFXTask */
osThreadId_t TouchGFXTaskHandle;
const osThreadAttr_t TouchGFXTask_attributes = {
  .name = "TouchGFXTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sdTask */
osThreadId_t sdTaskHandle;
const osThreadAttr_t sdTask_attributes = {
  .name = "sdTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static uint16_t DisplayTelemetry_ReadU16Le(const uint8_t *data);
static uint16_t DisplayTelemetry_ReadU16Be(const uint8_t *data);
static int16_t DisplayTelemetry_ReadS16Le(const uint8_t *data);
static int16_t DisplayTelemetry_ReadS16Be(const uint8_t *data);
static uint32_t DisplayTelemetry_ReadU32Le(const uint8_t *data);
static int32_t DisplayTelemetry_ReadS32Le(const uint8_t *data);
static uint8_t DisplayTelemetry_ApplySignalConfig(volatile DisplayTelemetry *telemetry, const DisplayTelemetrySignalConfig *signalConfig, const uint8_t *data, uint32_t length);
static uint8_t DisplayTelemetry_ProcessCanFrame(const DisplayTelemetryCanMessageConfig *messageConfig, const uint8_t *data, uint32_t length, uint32_t sourceCanId);
static void DisplayTelemetry_ConfigCanPort(FDCAN_HandleTypeDef *hfdcan);
static void SD_LogFatFsResult(const char *prefix, FRESULT result);
static void SD_TaskInit(void);
static void SD_TaskConsumePacket(const TelemetryFanoutPacket *packet);
static void SD_QueueCloseRequest(void);
static void SD_QueueConfigSaveRequest(void);
static uint8_t SD_EnsureMounted(void);
static void Config_LoadFromSd(void);
static uint8_t Config_ParseBytes(const uint8_t *data,
                                 UINT length,
                                 unsigned int *sdEnabled,
                                 unsigned int *alert0,
                                 unsigned int *alert1,
                                 unsigned int *alert2,
                                 unsigned int *alert3,
                                 unsigned int *alert4,
                                 unsigned int *nextLogSession);
static uint8_t Config_SaveToSd(void);
static FRESULT SD_OpenNextTelemetryLog(FIL *file);
static uint32_t DisplayTelemetry_DecodePayloadLength(uint32_t dlc);
static void Telemetry_FanoutSnapshot(uint32_t sourceCanId);
static void DisplayTelemetry_ProcessRxFrame(const CanRxFrame *frame);
static uint8_t DisplayConfig_ParseCanFrame(const CanRxFrame *frame, ConfigMessage *message);
static uint8_t LocalConfig_Apply(const ConfigMessage *message);
static uint8_t VehicleConfig_Apply(const ConfigMessage *message);
static uint8_t Config_ApplyMessage(const ConfigMessage *message);
static void Config_SendCanAck(const ConfigMessage *message, uint8_t applied);
static void CanTx_QueueMessage(const CanTxMessage *message);
static void CanTx_FillConfigHeader(FDCAN_TxHeaderTypeDef *header, uint32_t canId, uint32_t dlc);
static uint8_t VehicleConfig_IsAlertIndexValid(uint8_t index);
static uint8_t VehicleConfig_AlertIndexToKey(uint8_t index);
static uint8_t VehicleConfig_KeyToAlertIndex(uint8_t key, uint8_t *index);
static uint16_t VehicleConfig_ClampAlertValue(uint8_t index, uint16_t value);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTelemetryTask(void *argument);
void StartCanTxTask(void *argument);
void StartSdTask(void *argument);
extern void TouchGFX_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  canRxQueueHandle = osMessageQueueNew(CAN_RX_QUEUE_LENGTH, sizeof(CanRxFrame), NULL);
  canTxQueueHandle = osMessageQueueNew(CAN_TX_QUEUE_LENGTH, sizeof(CanTxMessage), NULL);
  sdLogQueueHandle = osMessageQueueNew(SD_LOG_QUEUE_LENGTH, sizeof(TelemetryFanoutPacket), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of telemetryTask */
  telemetryTaskHandle = osThreadNew(StartTelemetryTask, NULL, &telemetryTask_attributes);

  /* creation of canTxTask */
  canTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &canTxTask_attributes);

  /* creation of TouchGFXTask */
  TouchGFXTaskHandle = osThreadNew(TouchGFX_Task, NULL, &TouchGFXTask_attributes);

  /* creation of sdTask */
  sdTaskHandle = osThreadNew(StartSdTask, NULL, &sdTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  Debug_LogUart5((canRxQueueHandle != NULL) ? "[RTOS] canRxQueue created\r\n" : "[RTOS] canRxQueue create FAIL\r\n");
  Debug_LogUart5((canTxQueueHandle != NULL) ? "[RTOS] canTxQueue created\r\n" : "[RTOS] canTxQueue create FAIL\r\n");
  Debug_LogUart5((sdLogQueueHandle != NULL) ? "[RTOS] sdLogQueue created\r\n" : "[RTOS] sdLogQueue create FAIL\r\n");
  Debug_LogUart5((defaultTaskHandle != NULL) ? "[RTOS] defaultTask created\r\n" : "[RTOS] defaultTask create FAIL\r\n");
  Debug_LogUart5((telemetryTaskHandle != NULL) ? "[RTOS] telemetryTask created\r\n" : "[RTOS] telemetryTask create FAIL\r\n");
  Debug_LogUart5((canTxTaskHandle != NULL) ? "[RTOS] canTxTask created\r\n" : "[RTOS] canTxTask create FAIL\r\n");
  Debug_LogUart5((TouchGFXTaskHandle != NULL) ? "[RTOS] TouchGFXTask created\r\n" : "[RTOS] TouchGFXTask create FAIL\r\n");
  Debug_LogUart5((sdTaskHandle != NULL) ? "[RTOS] sdTask created\r\n" : "[RTOS] sdTask create FAIL\r\n");
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  uint32_t lastRuntimeLogTick = 0U;

  g_ltdc_stage = 12U;
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  Debug_LogUart5("[RTOS] defaultTask entered\r\n");
  Debug_LogUart5("[RTOS] scheduler alive\r\n");
  for(;;)
  {
    if (g_uiButtonsInitialized == 0U)
    {
      UIButtons_TaskInit();
      g_uiButtonsInitialized = 1U;
      Debug_LogUart5("[RTOS] ui buttons ready\r\n");
    }
    UIButtons_TaskPoll();
    if ((HAL_GetTick() - lastRuntimeLogTick) >= 1000U)
    {
      lastRuntimeLogTick = HAL_GetTick();
      DisplayDiag_LogRuntime();
    }

    osDelay(5U);
  }
  /* USER CODE END StartDefaultTask */
}

void StartTelemetryTask(void *argument)
{
  /* USER CODE BEGIN StartTelemetryTask */
  CanRxFrame frame;

  (void)argument;
  g_telemetryTaskEntered = 1U;
  Debug_LogUart5("[RTOS] telemetryTask entered\r\n");
  DisplayTelemetry_TaskInit();
  Debug_LogUart5("[RTOS] display telemetry task ready\r\n");

  for (;;)
  {
    if (osMessageQueueGet(canRxQueueHandle, &frame, NULL, 1000U) == osOK)
    {
      DisplayTelemetry_ProcessRxFrame(&frame);
      DisplayTelemetry_TaskStep();
    }
  }
  /* USER CODE END StartTelemetryTask */
}

void StartCanTxTask(void *argument)
{
  /* USER CODE BEGIN StartCanTxTask */
  CanTxMessage message;

  (void)argument;
  Debug_LogUart5("[RTOS] canTxTask entered\r\n");

  for (;;)
  {
    if (osMessageQueueGet(canTxQueueHandle, &message, NULL, osWaitForever) == osOK)
    {
      while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) == 0U)
      {
        osDelay(1U);
      }

      if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &message.header, message.data) != HAL_OK)
      {
        Debug_LogUart5("[CAN] tx enqueue to hw FAIL\r\n");
      }
    }
  }
  /* USER CODE END StartCanTxTask */
}

void StartSdTask(void *argument)
{
  /* USER CODE BEGIN StartSdTask */
  TelemetryFanoutPacket packet;

  (void)argument;
  Debug_LogUart5("[RTOS] sdTask entered\r\n");
  osDelay(2000U);
  SD_TaskInit();

  for (;;)
  {
    if (osMessageQueueGet(sdLogQueueHandle, &packet, NULL, 1000U) == osOK)
    {
      SD_TaskConsumePacket(&packet);
    }
    else if (g_configLoadDone == 0U)
    {
      Config_LoadFromSd();
    }
  }
  /* USER CODE END StartSdTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static void SD_LogFatFsResult(const char *prefix, FRESULT result)
{
  char buffer[64];

  (void)snprintf(buffer,
                 sizeof(buffer),
                 "[SD] %s -> FR=%lu\r\n",
                 (prefix != NULL) ? prefix : "op",
                 (unsigned long)result);
  Debug_LogUart5(buffer);
}

static void SD_TaskInit(void)
{
  Config_LoadFromSd();
}

static uint8_t SD_EnsureMounted(void)
{
  FRESULT fr;

  if (g_sdFatFsMounted != 0U)
  {
    return 1U;
  }

  fr = f_mount(&SDFatFS, SDPath, 1);
  if (fr != FR_OK)
  {
    sdmmc1_init_ok = 0U;
    SD_LogFatFsResult("mount fail", fr);
    osDelay(1000U);
    return 0U;
  }

  sdmmc1_init_ok = 1U;
  g_sdFatFsMounted = 1U;
  return 1U;
}

static uint8_t Config_ParseBytes(const uint8_t *data,
                                 UINT length,
                                 unsigned int *sdEnabled,
                                 unsigned int *alert0,
                                 unsigned int *alert1,
                                 unsigned int *alert2,
                                 unsigned int *alert3,
                                 unsigned int *alert4,
                                 unsigned int *nextLogSession)
{
  unsigned int values[7];
  unsigned int bestValues[7];
  uint8_t count = 0U;
  uint8_t bestCount = 0U;
  uint8_t lineHasText = 0U;
  UINT i;

  if ((data == NULL) ||
      (sdEnabled == NULL) ||
      (alert0 == NULL) ||
      (alert1 == NULL) ||
      (alert2 == NULL) ||
      (alert3 == NULL) ||
      (alert4 == NULL) ||
      (nextLogSession == NULL))
  {
    return 0U;
  }

  values[6] = *nextLogSession;
  bestValues[6] = *nextLogSession;
  for (i = 0U; i < length; ++i)
  {
    if (((data[i] >= (uint8_t)'A') && (data[i] <= (uint8_t)'Z')) ||
        ((data[i] >= (uint8_t)'a') && (data[i] <= (uint8_t)'z')))
    {
      lineHasText = 1U;
    }
    else if ((count < 7U) && (data[i] >= (uint8_t)'0') && (data[i] <= (uint8_t)'9'))
    {
      unsigned int value = 0U;

      do
      {
        value = (value * 10U) + (unsigned int)(data[i] - (uint8_t)'0');
        ++i;
        while ((i < length) && (data[i] == 0U))
        {
          ++i;
        }
      } while ((i < length) && (data[i] >= (uint8_t)'0') && (data[i] <= (uint8_t)'9'));

      values[count++] = value;
    }

    if ((data[i] == (uint8_t)'\n') || (data[i] == (uint8_t)'\r'))
    {
      if ((lineHasText == 0U) && (count >= 6U))
      {
        for (uint8_t j = 0U; j < 7U; ++j)
        {
          bestValues[j] = values[j];
        }
        bestCount = count;
      }
      values[6] = *nextLogSession;
      count = 0U;
      lineHasText = 0U;
    }
  }

  if ((lineHasText == 0U) && (count >= 6U))
  {
    for (uint8_t j = 0U; j < 7U; ++j)
    {
      bestValues[j] = values[j];
    }
    bestCount = count;
  }

  if (bestCount < 6U)
  {
    return 0U;
  }

  *sdEnabled = bestValues[0];
  *alert0 = bestValues[1];
  *alert1 = bestValues[2];
  *alert2 = bestValues[3];
  *alert3 = bestValues[4];
  *alert4 = bestValues[5];
  *nextLogSession = bestValues[6];
  return 1U;
}

static void Config_LoadFromSd(void)
{
  FIL file;
  uint8_t fileBuffer[256];
  UINT bytesRead = 0U;
  unsigned int sdEnabled;
  unsigned int alert0;
  unsigned int alert1;
  unsigned int alert2;
  unsigned int alert3;
  unsigned int alert4;
  unsigned int nextLogSession = 0U;
  FRESULT fr;

  if (SD_EnsureMounted() == 0U)
  {
    return;
  }

  fr = f_open(&file, CONFIG_FILE_PATH, FA_READ);
  if (fr != FR_OK)
  {
    if (fr == FR_NO_FILE)
    {
      g_configLoadDone = Config_SaveToSd();
    }
    return;
  }

  fr = f_read(&file, fileBuffer, sizeof(fileBuffer), &bytesRead);
  if ((fr == FR_OK) &&
      (Config_ParseBytes(fileBuffer, bytesRead, &sdEnabled, &alert0, &alert1, &alert2, &alert3, &alert4, &nextLogSession) != 0U))
  {
    g_localConfig.sdLoggingEnabled = (sdEnabled != 0U) ? 1U : 0U;
    g_localConfig.nextLogSession = (uint16_t)(nextLogSession % 1000U);
    g_vehicleConfig.alertThresholds[0] = VehicleConfig_ClampAlertValue(0U, (uint16_t)alert0);
    g_vehicleConfig.alertThresholds[1] = VehicleConfig_ClampAlertValue(1U, (uint16_t)alert1);
    g_vehicleConfig.alertThresholds[2] = VehicleConfig_ClampAlertValue(2U, (uint16_t)alert2);
    g_vehicleConfig.alertThresholds[3] = VehicleConfig_ClampAlertValue(3U, (uint16_t)alert3);
    g_vehicleConfig.alertThresholds[4] = VehicleConfig_ClampAlertValue(4U, (uint16_t)alert4);
  }
  else
  {
    (void)f_close(&file);
    return;
  }

  (void)f_close(&file);
  g_configLoadDone = 1U;
}

static uint8_t Config_SaveToSd(void)
{
  FIL file;
  UINT written = 0U;
  char buffer[160];
  UINT length;
  FRESULT fr;

  if (SD_EnsureMounted() == 0U)
  {
    return 0U;
  }

  length = (UINT)snprintf(buffer,
                          sizeof(buffer),
                          "sd_logging_enabled,alert_motor_temp,alert_inverter_temp,alert_accu_temp,alert_soc,alert_accu_min_voltage,next_log_session\r\n"
                          "%u,%u,%u,%u,%u,%u,%u\r\n",
                          (unsigned)g_localConfig.sdLoggingEnabled,
                          (unsigned)g_vehicleConfig.alertThresholds[0],
                          (unsigned)g_vehicleConfig.alertThresholds[1],
                          (unsigned)g_vehicleConfig.alertThresholds[2],
                          (unsigned)g_vehicleConfig.alertThresholds[3],
                          (unsigned)g_vehicleConfig.alertThresholds[4],
                          (unsigned)g_localConfig.nextLogSession);
  if ((length == 0U) || (length >= sizeof(buffer)))
  {
    return 0U;
  }

  fr = f_open(&file, CONFIG_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if (fr != FR_OK)
  {
    SD_LogFatFsResult("open config fail", fr);
    return 0U;
  }

  fr = f_write(&file, buffer, length, &written);
  if ((fr != FR_OK) || (written != length))
  {
    SD_LogFatFsResult("write config fail", fr);
    (void)f_close(&file);
    return 0U;
  }

  fr = f_close(&file);
  if (fr != FR_OK)
  {
    SD_LogFatFsResult("close config fail", fr);
    return 0U;
  }

  return 1U;
}

static FRESULT SD_OpenNextTelemetryLog(FIL *file)
{
  FRESULT fr;
  uint16_t session;
  uint16_t attempt;
  char sessionId[13];
  char filePath[18];

  if (file == NULL)
  {
    return FR_INVALID_OBJECT;
  }

  session = (uint16_t)(g_localConfig.nextLogSession % 1000U);

  for (attempt = 0U; attempt < 1000U; attempt++)
  {
    (void)snprintf(sessionId,
                   sizeof(sessionId),
                   "TELOG%03u.CSV",
                   (unsigned)session);
    (void)snprintf(filePath,
                   sizeof(filePath),
                   "0:/%s",
                   sessionId);

    fr = f_open(file, filePath, FA_CREATE_NEW | FA_WRITE);
    if (fr == FR_OK)
    {
      (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "%s", sessionId);
      (void)snprintf(g_sdLogFilePath, sizeof(g_sdLogFilePath), "%s", filePath);
      g_localConfig.nextLogSession = (uint16_t)((session + 1U) % 1000U);
      return FR_OK;
    }

    if (fr != FR_EXIST)
    {
      (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "NOLOG");
      g_sdLogFilePath[0] = '\0';
      return fr;
    }

    session = (uint16_t)((session + 1U) % 1000U);
  }

  (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "NOLOG");
  return FR_DENIED;
}

static void SD_TaskConsumePacket(const TelemetryFanoutPacket *packet)
{
  static uint8_t fileReady = 0U;
  FRESULT fr;
  UINT bytesProcessed = 0U;
  char lineBuffer[512];

  if (packet == NULL)
  {
    return;
  }

  if ((g_configLoadDone == 0U) && (packet->configSaveRequest == 0U))
  {
    Config_LoadFromSd();
  }

  if (packet->configSaveRequest != 0U)
  {
    Config_SaveToSd();
    return;
  }

  if (packet->closeRequest != 0U)
  {
    fileReady = 0U;
    g_sdLogRecordingActive = 0U;
    return;
  }

  if (g_localConfig.sdLoggingEnabled == 0U)
  {
    g_sdLogRecordingActive = 0U;
    g_sdLogCreateRequested = 0U;
    return;
  }

  if ((fileReady == 0U) &&
      (g_sdLogCreateRequested == 0U) &&
      (g_sdLogFilePath[0] == '\0'))
  {
    g_sdLogRecordingActive = 0U;
    return;
  }

  if (g_sdFatFsMounted == 0U)
  {
    if (SD_EnsureMounted() == 0U)
    {
      g_sdLogRecordingActive = 0U;
      return;
    }
  }

  if (fileReady == 0U)
  {
    if (g_sdLogCreateRequested == 0U)
    {
      fileReady = 1U;
      g_sdLogRecordingActive = 1U;
    }
  }

  if (fileReady == 0U)
  {
    FIL logFile;
    fr = SD_OpenNextTelemetryLog(&logFile);
    if (fr != FR_OK)
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("open log fail", fr);
      g_sdFatFsMounted = 0U;
      return;
    }

    static const char header[] = "#TELOG\r\n";
    UINT headerWritten = 0U;

    fr = f_write(&logFile, header, sizeof(header) - 1U, &headerWritten);
    if ((fr != FR_OK) || (headerWritten != (sizeof(header) - 1U)))
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("header write fail", fr);
      (void)f_close(&logFile);
      (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "NOLOG");
      g_sdLogFilePath[0] = '\0';
      g_sdFatFsMounted = 0U;
      return;
    }

    fr = f_close(&logFile);
    if (fr != FR_OK)
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("header close fail", fr);
      fileReady = 0U;
      (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "NOLOG");
      g_sdLogFilePath[0] = '\0';
      g_sdFatFsMounted = 0U;
      return;
    }

    fileReady = 1U;
    g_sdLogCreateRequested = 0U;
    g_configLoadDone = Config_SaveToSd();
    g_sdLogRecordingActive = (g_localConfig.sdLoggingEnabled != 0U) ? 1U : 0U;
  }

  bytesProcessed = (UINT)snprintf(lineBuffer,
                                  sizeof(lineBuffer),
                                  "%lu,%u,"
                                  "%u,%u,%u,%u,%u,%u,%u,%u,"
                                  "%u,%u,%u,%u,"
                                  "%u,%u,%u,%u,%u,"
                                  "%u,%u,%u,%u,%u,"
                                  "%d,%d,%d,"
                                  "%d,%d,%d,%d,%d,"
                                  "%u,%u,%u,%u,"
                                  "%d,%d,%d,%ld,%ld,%ld,"
                                  "%u,%u,%ld,%u,%u,%u,%ld,%ld\r\n",
                                  (unsigned long)packet->snapshot.tick_ms,
                                  (unsigned)packet->snapshot.sequence,
                                  (unsigned)packet->snapshot.ecu_fsm_state,
                                  (unsigned)packet->snapshot.ecu_boton_arranque,
                                  (unsigned)packet->snapshot.ecu_s1_aceleracion,
                                  (unsigned)packet->snapshot.ecu_s2_aceleracion,
                                  (unsigned)packet->snapshot.ecu_s_freno,
                                  (unsigned)packet->snapshot.ecu_torque_total,
                                  (unsigned)packet->snapshot.ecu_flag_ev_2_3,
                                  (unsigned)packet->snapshot.ecu_flag_t11_8_9,
                                  (unsigned)packet->snapshot.ams_ok_precarga,
                                  (unsigned)packet->snapshot.ams_state,
                                  (unsigned)packet->snapshot.ams_v_celda_min,
                                  (unsigned)packet->snapshot.ams_soc,
                                  (unsigned)packet->snapshot.ams_vmin_modulo[0],
                                  (unsigned)packet->snapshot.ams_vmin_modulo[1],
                                  (unsigned)packet->snapshot.ams_vmin_modulo[2],
                                  (unsigned)packet->snapshot.ams_vmin_modulo[3],
                                  (unsigned)packet->snapshot.ams_vmin_modulo[4],
                                  (unsigned)packet->snapshot.ams_vmax_modulo[0],
                                  (unsigned)packet->snapshot.ams_vmax_modulo[1],
                                  (unsigned)packet->snapshot.ams_vmax_modulo[2],
                                  (unsigned)packet->snapshot.ams_vmax_modulo[3],
                                  (unsigned)packet->snapshot.ams_vmax_modulo[4],
                                  (int)packet->snapshot.ams_corriente_accu,
                                  (int)packet->snapshot.ams_corriente_dcdc,
                                  (int)packet->snapshot.ams_temp_dcdc,
                                  (int)packet->snapshot.ams_temp_max_modulo[0],
                                  (int)packet->snapshot.ams_temp_max_modulo[1],
                                  (int)packet->snapshot.ams_temp_max_modulo[2],
                                  (int)packet->snapshot.ams_temp_max_modulo[3],
                                  (int)packet->snapshot.ams_temp_max_modulo[4],
                                  (unsigned)packet->snapshot.inverter_inv_state,
                                  (unsigned)packet->snapshot.inverter_inv_vdc_ready,
                                  (unsigned)packet->snapshot.inverter_inv_error,
                                  (unsigned)packet->snapshot.inverter_inv_dc_bus_voltage,
                                  (int)packet->snapshot.inverter_inv_motor_temp,
                                  (int)packet->snapshot.inverter_inv_igbt_temp,
                                  (int)packet->snapshot.inverter_inv_air_temp,
                                  (long)packet->snapshot.inverter_inv_rpm,
                                  (long)packet->snapshot.inverter_inv_speed_actual,
                                  (long)packet->snapshot.inverter_inv_current_actual,
                                  (unsigned)packet->snapshot.gps_speed,
                                  (unsigned)packet->snapshot.gps_course_deg,
                                  (long)packet->snapshot.gps_altitude,
                                  (unsigned)packet->snapshot.gps_fix_type,
                                  (unsigned)packet->snapshot.gps_sat_count,
                                  (unsigned)packet->snapshot.gps_hdop,
                                  (long)packet->snapshot.gps_latitude,
                                  (long)packet->snapshot.gps_longitude);

  if ((bytesProcessed == 0U) || (bytesProcessed >= sizeof(lineBuffer)))
  {
    Debug_LogUart5("[SD] telemetry row truncated\r\n");
    return;
  }

  {
    FIL logFile;
    UINT lineWritten = 0U;
    fr = f_open(&logFile, g_sdLogFilePath, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("open log append fail", fr);
      fileReady = 0U;
      g_sdFatFsMounted = 0U;
      return;
    }

    fr = f_write(&logFile, lineBuffer, bytesProcessed, &lineWritten);
    if ((fr != FR_OK) || (lineWritten != bytesProcessed))
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("log write fail", fr);
      (void)f_close(&logFile);
      fileReady = 0U;
      g_sdFatFsMounted = 0U;
      return;
    }

    fr = f_close(&logFile);
    if (fr != FR_OK)
    {
      g_sdLogRecordingActive = 0U;
      SD_LogFatFsResult("log close fail", fr);
      fileReady = 0U;
      g_sdFatFsMounted = 0U;
      return;
    }
  }

  g_sdLogRecordingActive = (g_localConfig.sdLoggingEnabled != 0U) ? 1U : 0U;
}

static uint16_t DisplayTelemetry_ReadU16Le(const uint8_t *data)
{
  return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static uint16_t DisplayTelemetry_ReadU16Be(const uint8_t *data)
{
  return (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
}

static int16_t DisplayTelemetry_ReadS16Le(const uint8_t *data)
{
  return (int16_t)DisplayTelemetry_ReadU16Le(data);
}

static int16_t DisplayTelemetry_ReadS16Be(const uint8_t *data)
{
  return (int16_t)DisplayTelemetry_ReadU16Be(data);
}

static uint32_t DisplayTelemetry_ReadU32Le(const uint8_t *data)
{
  return ((uint32_t)data[0]) |
         ((uint32_t)data[1] << 8) |
         ((uint32_t)data[2] << 16) |
         ((uint32_t)data[3] << 24);
}

static int32_t DisplayTelemetry_ReadS32Le(const uint8_t *data)
{
  return (int32_t)DisplayTelemetry_ReadU32Le(data);
}

static uint8_t DisplayTelemetry_ApplySignalConfig(volatile DisplayTelemetry *telemetry, const DisplayTelemetrySignalConfig *signalConfig, const uint8_t *data, uint32_t length)
{
  volatile uint8_t *telemetryBytes = NULL;
  volatile void *destination = NULL;
  uint32_t rawUnsignedValue = 0U;
  int32_t rawSignedValue = 0;
  uint8_t isSignedValue = 0U;

  if ((telemetry == NULL) || (signalConfig == NULL) || (data == NULL))
  {
    return 0U;
  }

  if ((length < signalConfig->minPayloadLength) || (length > signalConfig->maxPayloadLength))
  {
    return 0U;
  }

  if ((signalConfig->telemetryOffset + signalConfig->telemetrySize) > sizeof(DisplayTelemetry))
  {
    return 0U;
  }

  telemetryBytes = (volatile uint8_t *)telemetry;
  destination = (volatile void *)(telemetryBytes + signalConfig->telemetryOffset);

  switch (signalConfig->encoding)
  {
    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8:
      if ((signalConfig->payloadOffset + 1U) > length)
      {
        return 0U;
      }
      rawUnsignedValue = data[signalConfig->payloadOffset];
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_BIT:
      if ((signalConfig->payloadOffset + 1U) > length)
      {
        return 0U;
      }
      rawUnsignedValue = ((data[signalConfig->payloadOffset] & signalConfig->bitMask) != 0U) ? 1U : 0U;
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE:
      if ((signalConfig->payloadOffset + 2U) > length)
      {
        return 0U;
      }
      rawUnsignedValue = (uint32_t)DisplayTelemetry_ReadU16Le(&data[signalConfig->payloadOffset]);
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_BE:
      if ((signalConfig->payloadOffset + 2U) > length)
      {
        return 0U;
      }
      rawUnsignedValue = (uint32_t)DisplayTelemetry_ReadU16Be(&data[signalConfig->payloadOffset]);
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE:
      if ((signalConfig->payloadOffset + 2U) > length)
      {
        return 0U;
      }
      rawSignedValue = (int32_t)DisplayTelemetry_ReadS16Le(&data[signalConfig->payloadOffset]);
      isSignedValue = 1U;
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_BE:
      if ((signalConfig->payloadOffset + 2U) > length)
      {
        return 0U;
      }
      rawSignedValue = (int32_t)DisplayTelemetry_ReadS16Be(&data[signalConfig->payloadOffset]);
      isSignedValue = 1U;
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_U32_LE:
      if ((signalConfig->payloadOffset + 4U) > length)
      {
        return 0U;
      }
      rawUnsignedValue = DisplayTelemetry_ReadU32Le(&data[signalConfig->payloadOffset]);
      break;

    case DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE:
      if ((signalConfig->payloadOffset + 4U) > length)
      {
        return 0U;
      }
      rawSignedValue = DisplayTelemetry_ReadS32Le(&data[signalConfig->payloadOffset]);
      isSignedValue = 1U;
      break;

    default:
      return 0U;
  }

  if (isSignedValue != 0U)
  {
    switch (signalConfig->telemetrySize)
    {
      case 1U:
        *(volatile int8_t *)destination = (int8_t)rawSignedValue;
        break;

      case 2U:
        *(volatile int16_t *)destination = (int16_t)rawSignedValue;
        break;

      case 4U:
        *(volatile int32_t *)destination = rawSignedValue;
        break;

      default:
        return 0U;
    }
  }
  else
  {
    switch (signalConfig->telemetrySize)
    {
      case 1U:
        *(volatile uint8_t *)destination = (uint8_t)rawUnsignedValue;
        break;

      case 2U:
        *(volatile uint16_t *)destination = (uint16_t)rawUnsignedValue;
        break;

      case 4U:
        *(volatile uint32_t *)destination = rawUnsignedValue;
        break;

      default:
        return 0U;
    }
  }

  return 1U;
}

void DisplayTelemetry_Reset(void)
{
  (void)memset((void *)&g_displayTelemetry, 0, sizeof(g_displayTelemetry));
  g_displayTelemetryWriteSequence = 0U;
  g_displayTelemetrySnapshotSequence = 0U;
  g_displayTelemetryValid = 0U;
  g_displayTelemetryPendingMask = 0U;
}

void DisplayTelemetry_TaskInit(void)
{
  g_displayCanReady = 0U;
  DisplayTelemetry_ConfigCanPort(&hfdcan2);
  DisplayTelemetry_Reset();
  g_displayCanReady = 1U;
  Debug_LogUart5("[CAN] FDCAN2 display telemetry ready\r\n");
}

static void DisplayTelemetry_ConfigCanPort(FDCAN_HandleTypeDef *hfdcan)
{
  FDCAN_FilterTypeDef filter = {0};
  FDCAN_FilterTypeDef configFilter = {0};
  uint32_t lowestAcceptedId = DisplayTelemetryCanConfig_GetMinId();
  uint32_t highestAcceptedId = DisplayTelemetryCanConfig_GetMaxId();

  if (hfdcan == NULL)
  {
    Error_Handler();
  }

  configFilter.IdType = FDCAN_STANDARD_ID;
  configFilter.FilterIndex = 0;
  configFilter.FilterType = FDCAN_FILTER_MASK;
  configFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  configFilter.FilterID1 = CAN_CONFIG_RX_ID;
  configFilter.FilterID2 = 0x7FFU;

  if (HAL_FDCAN_ConfigFilter(hfdcan, &configFilter) != HAL_OK)
  {
    Error_Handler();
  }

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 1;
  filter.FilterType = FDCAN_FILTER_RANGE;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = lowestAcceptedId;
  filter.FilterID2 = highestAcceptedId;

  if (HAL_FDCAN_ConfigFilter(hfdcan, &filter) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U) != HAL_OK)
  {
    Error_Handler();
  }
}

void DisplayTelemetry_TaskStep(void)
{
  CanRxFrame frame;

  while (osMessageQueueGet(canRxQueueHandle, &frame, NULL, 0U) == osOK)
  {
    DisplayTelemetry_ProcessRxFrame(&frame);
  }
}

static uint8_t DisplayTelemetry_ProcessCanFrame(const DisplayTelemetryCanMessageConfig *messageConfig, const uint8_t *data, uint32_t length, uint32_t sourceCanId)
{
  uint32_t appliedSignalCount = 0U;
  uint32_t receivedMaskBit = 0U;

  if ((messageConfig == NULL) || (data == NULL) || (length == 0U))
  {
    return 0U;
  }

  g_displayTelemetryWriteSequence++;
  __DMB();

  for (uint32_t signalIndex = 0U; signalIndex < messageConfig->signalCount; signalIndex++)
  {
    appliedSignalCount += DisplayTelemetry_ApplySignalConfig(&g_displayTelemetry,
                                                             &messageConfig->signals[signalIndex],
                                                             data,
                                                             length);
  }

  if (appliedSignalCount == 0U)
  {
    __DMB();
    g_displayTelemetryWriteSequence++;
    return 0U;
  }

  receivedMaskBit = messageConfig->snapshotMask;
  g_displayTelemetryPendingMask |= receivedMaskBit;
  g_displayTelemetryValid = 1U;
  g_displayTelemetrySnapshotSequence++;

  if (g_displayTelemetryPendingMask == DisplayTelemetryCanConfig_GetSnapshotMaskAll())
  {
    g_displayTelemetryPendingMask = 0U;
  }

  __DMB();
  g_displayTelemetryWriteSequence++;
  Telemetry_FanoutSnapshot(sourceCanId);

  return 1U;
}

uint8_t DisplayTelemetry_GetSnapshot(DisplayTelemetry *snapshot, uint32_t *sequence)
{
  DisplayTelemetry localSnapshot;
  uint32_t writeSequenceStart = 0U;
  uint32_t writeSequenceEnd = 0U;
  uint32_t snapshotSequence = 0U;

  if ((snapshot == NULL) || (g_displayTelemetryValid == 0U))
  {
    return 0U;
  }

  do
  {
    writeSequenceStart = g_displayTelemetryWriteSequence;
    if ((writeSequenceStart & 1U) != 0U)
    {
      continue;
    }

    __DMB();
    snapshotSequence = g_displayTelemetrySnapshotSequence;
    localSnapshot = g_displayTelemetry;
    __DMB();
    writeSequenceEnd = g_displayTelemetryWriteSequence;
  } while ((writeSequenceStart != writeSequenceEnd) || ((writeSequenceEnd & 1U) != 0U));

  if (snapshotSequence == 0U)
  {
    return 0U;
  }

  *snapshot = localSnapshot;

  if (sequence != NULL)
  {
    *sequence = snapshotSequence;
  }

  return 1U;
}

static uint32_t DisplayTelemetry_DecodePayloadLength(uint32_t dlc)
{
  switch (dlc)
  {
    case FDCAN_DLC_BYTES_0:
      return 0U;
    case FDCAN_DLC_BYTES_1:
      return 1U;
    case FDCAN_DLC_BYTES_2:
      return 2U;
    case FDCAN_DLC_BYTES_3:
      return 3U;
    case FDCAN_DLC_BYTES_4:
      return 4U;
    case FDCAN_DLC_BYTES_5:
      return 5U;
    case FDCAN_DLC_BYTES_6:
      return 6U;
    case FDCAN_DLC_BYTES_7:
      return 7U;
    case FDCAN_DLC_BYTES_8:
      return 8U;
    default:
      return 0U;
  }
}

static void Telemetry_FanoutSnapshot(uint32_t sourceCanId)
{
  static uint32_t lastSdLogTick = 0U;
  TelemetryFanoutPacket packet = {0};
  uint32_t now = HAL_GetTick();

  if (DisplayTelemetry_GetSnapshot(&packet.snapshot, &packet.sequence) == 0U)
  {
    return;
  }

  packet.tick = now;
  packet.sourceCanId = sourceCanId;
  packet.closeRequest = 0U;

  if ((sdLogQueueHandle != NULL) &&
      (g_localConfig.sdLoggingEnabled != 0U) &&
      ((g_sdLogRecordingActive != 0U) ||
       (g_sdLogCreateRequested != 0U) ||
       (g_sdLogFilePath[0] != '\0')))
  {
    if (((now - lastSdLogTick) < SD_LOG_MIN_PERIOD_MS) ||
        (osMessageQueueGetCount(sdLogQueueHandle) != 0U))
    {
      return;
    }

    if (osMessageQueuePut(sdLogQueueHandle, &packet, 0U, 0U) == osOK)
    {
      lastSdLogTick = now;
      g_sdLogQueuedFrames++;
    }
    else
    {
      g_sdLogDroppedFrames++;
    }
  }
}

static void SD_QueueCloseRequest(void)
{
  TelemetryFanoutPacket packet = {0};

  packet.closeRequest = 1U;
  if (sdLogQueueHandle == NULL)
  {
    return;
  }

  (void)osMessageQueueReset(sdLogQueueHandle);
  if (osMessageQueuePut(sdLogQueueHandle, &packet, 0U, 0U) != osOK)
  {
    g_sdLogDroppedFrames++;
  }
}

static void SD_QueueConfigSaveRequest(void)
{
  TelemetryFanoutPacket packet = {0};

  packet.configSaveRequest = 1U;
  if (sdLogQueueHandle == NULL)
  {
    return;
  }

  (void)osMessageQueueReset(sdLogQueueHandle);
  if (osMessageQueuePut(sdLogQueueHandle, &packet, 0U, 0U) != osOK)
  {
    g_sdLogDroppedFrames++;
  }
}

static void DisplayTelemetry_ProcessRxFrame(const CanRxFrame *frame)
{
  const DisplayTelemetryCanMessageConfig *messageConfig = NULL;
  ConfigMessage configMessage;

  if (frame == NULL)
  {
    return;
  }

  if (DisplayConfig_ParseCanFrame(frame, &configMessage) != 0U)
  {
    (void)Config_ApplyMessage(&configMessage);
    return;
  }

  messageConfig = DisplayTelemetryCanConfig_FindById(frame->header.Identifier);

  if ((frame->header.IdType == FDCAN_STANDARD_ID) &&
      (frame->header.RxFrameType == FDCAN_DATA_FRAME) &&
      (messageConfig != NULL) &&
      (frame->length >= messageConfig->minPayloadLength) &&
      (frame->length <= messageConfig->maxPayloadLength))
  {
    (void)DisplayTelemetry_ProcessCanFrame(messageConfig, frame->data, frame->length, frame->header.Identifier);
  }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if ((hfdcan == NULL) ||
      (hfdcan->Instance != FDCAN2) ||
      ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U))
  {
    return;
  }

  while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0U)
  {
    CanRxFrame frame = {0};

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &frame.header, frame.data) != HAL_OK)
    {
      break;
    }

    frame.length = (uint8_t)DisplayTelemetry_DecodePayloadLength(frame.header.DataLength);
    frame.tick = HAL_GetTick();
    g_canRxIsrCount++;

    if (osMessageQueuePut(canRxQueueHandle, &frame, 0U, 0U) != osOK)
    {
      g_canRxDroppedFrames++;
    }
  }
}

static uint8_t DisplayConfig_ParseCanFrame(const CanRxFrame *frame, ConfigMessage *message)
{
  if ((frame == NULL) || (message == NULL))
  {
    return 0U;
  }

  if ((frame->header.Identifier != CAN_CONFIG_RX_ID) || (frame->length < 2U))
  {
    return 0U;
  }

  message->source = CONFIG_SOURCE_CAN;
  message->key = frame->data[0];
  message->value = frame->data[1];
  if (frame->length >= 4U)
  {
    message->value |= (uint16_t)((uint16_t)frame->data[2] << 8);
  }
  message->originId = frame->header.Identifier;
  message->tick = frame->tick;
  return 1U;
}

static uint8_t LocalConfig_Apply(const ConfigMessage *message)
{
  if (message == NULL)
  {
    return 0U;
  }

  switch (message->key)
  {
    case APP_CONFIG_KEY_SD_LOG_ENABLE:
      g_localConfig.sdLoggingEnabled = (message->value != 0U) ? 1U : 0U;
      if (g_localConfig.sdLoggingEnabled == 0U)
      {
        g_sdLogRecordingActive = 0U;
        g_sdLogCreateRequested = 0U;
        SD_QueueCloseRequest();
      }
      return 1U;

    default:
      return 0U;
  }
}

static uint8_t VehicleConfig_Apply(const ConfigMessage *message)
{
  uint8_t index;

  if ((message == NULL) || (VehicleConfig_KeyToAlertIndex(message->key, &index) == 0U))
  {
    return 0U;
  }

  g_vehicleConfig.alertThresholds[index] = VehicleConfig_ClampAlertValue(index, message->value);
  return 1U;
}

static uint8_t Config_ApplyMessage(const ConfigMessage *message)
{
  uint8_t applied = 0U;
  char buffer[96];

  if (message == NULL)
  {
    return 0U;
  }

  applied = LocalConfig_Apply(message);
  if (applied == 0U)
  {
    applied = VehicleConfig_Apply(message);
  }

  (void)snprintf(buffer,
                 sizeof(buffer),
                 "[CFG] src=%u key=%u value=%u applied=%u\r\n",
                 (unsigned)message->source,
                 (unsigned)message->key,
                 (unsigned)message->value,
                 (unsigned)applied);
  Debug_LogUart5(buffer);

  if (message->source == CONFIG_SOURCE_CAN)
  {
    Config_SendCanAck(message, applied);
  }

  if (applied != 0U)
  {
    SD_QueueConfigSaveRequest();
  }

  return applied;
}

static void Config_SendCanAck(const ConfigMessage *message, uint8_t applied)
{
  CanTxMessage txMessage;

  if (message == NULL)
  {
    return;
  }

  (void)memset(&txMessage, 0, sizeof(txMessage));
  CanTx_FillConfigHeader(&txMessage.header, CAN_CONFIG_TX_ID, FDCAN_DLC_BYTES_4);
  txMessage.length = 4U;
  txMessage.data[0] = message->key;
  txMessage.data[1] = (uint8_t)(message->value & 0xFFU);
  txMessage.data[2] = (uint8_t)((message->value >> 8) & 0xFFU);
  txMessage.data[3] = applied;
  CanTx_QueueMessage(&txMessage);
}

static void CanTx_QueueMessage(const CanTxMessage *message)
{
  if (message == NULL)
  {
    return;
  }

  if (osMessageQueuePut(canTxQueueHandle, message, 0U, 0U) != osOK)
  {
    g_canTxDroppedFrames++;
  }
}

static void CanTx_FillConfigHeader(FDCAN_TxHeaderTypeDef *header, uint32_t canId, uint32_t dlc)
{
  if (header == NULL)
  {
    return;
  }

  header->Identifier = canId;
  header->IdType = FDCAN_STANDARD_ID;
  header->TxFrameType = FDCAN_DATA_FRAME;
  header->DataLength = dlc;
  header->ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  header->BitRateSwitch = FDCAN_BRS_OFF;
  header->FDFormat = FDCAN_CLASSIC_CAN;
  header->TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  header->MessageMarker = 0U;
}

static uint8_t VehicleConfig_IsAlertIndexValid(uint8_t index)
{
  return (index < VEHICLE_CONFIG_ALERT_PARAM_COUNT) ? 1U : 0U;
}

static uint8_t VehicleConfig_AlertIndexToKey(uint8_t index)
{
  return (uint8_t)(APP_CONFIG_KEY_VEHICLE_ALERT_0 + index);
}

static uint8_t VehicleConfig_KeyToAlertIndex(uint8_t key, uint8_t *index)
{
  if ((index == NULL) ||
      (key < APP_CONFIG_KEY_VEHICLE_ALERT_0) ||
      (key > APP_CONFIG_KEY_VEHICLE_ALERT_4))
  {
    return 0U;
  }

  *index = (uint8_t)(key - APP_CONFIG_KEY_VEHICLE_ALERT_0);
  return 1U;
}

static uint16_t VehicleConfig_ClampAlertValue(uint8_t index, uint16_t value)
{
  static const uint16_t maxValues[VEHICLE_CONFIG_ALERT_PARAM_COUNT] =
  {
    VEHICLE_CONFIG_ALERT_0_MAX,
    VEHICLE_CONFIG_ALERT_1_MAX,
    VEHICLE_CONFIG_ALERT_2_MAX,
    VEHICLE_CONFIG_ALERT_3_MAX,
    VEHICLE_CONFIG_ALERT_4_MAX
  };

  if (VehicleConfig_IsAlertIndexValid(index) == 0U)
  {
    return 0U;
  }

  return (value > maxValues[index]) ? maxValues[index] : value;
}

uint8_t AppConfig_IsLocalSdLoggingEnabled(void)
{
  return g_localConfig.sdLoggingEnabled;
}

uint8_t AppConfig_IsSdLogRecordingActive(void)
{
  return g_sdLogRecordingActive;
}

uint8_t AppConfig_SetLocalSdLoggingEnabled(uint8_t enabled)
{
  ConfigMessage message = {0};

  message.source = CONFIG_SOURCE_LOCAL;
  message.key = APP_CONFIG_KEY_SD_LOG_ENABLE;
  message.value = (enabled != 0U) ? 1U : 0U;
  message.originId = 0U;
  message.tick = HAL_GetTick();
  return Config_ApplyMessage(&message);
}

void AppConfig_RequestNewSdLogSession(void)
{
  if (g_localConfig.sdLoggingEnabled == 0U)
  {
    return;
  }

  g_sdLogCreateRequested = 1U;
  g_sdLogRecordingActive = 0U;
  g_sdLogFilePath[0] = '\0';
  (void)snprintf(g_sdLogSessionId, sizeof(g_sdLogSessionId), "NOLOG");
  SD_QueueCloseRequest();
}

void AppConfig_CopySdLogSessionId(char *buffer, uint32_t bufferSize)
{
  if ((buffer == NULL) || (bufferSize == 0U))
  {
    return;
  }

  taskENTER_CRITICAL();
  (void)snprintf(buffer, bufferSize, "%s", g_sdLogSessionId);
  taskEXIT_CRITICAL();
}

void AppConfig_GetVehicleConfigParams(VehicleConfigParams *params)
{
  if (params == NULL)
  {
    return;
  }

  *params = *(const VehicleConfigParams *)(const void *)&g_vehicleConfig;
}

uint16_t AppConfig_GetVehicleAlertThreshold(uint8_t index)
{
  if (VehicleConfig_IsAlertIndexValid(index) == 0U)
  {
    return 0U;
  }

  return g_vehicleConfig.alertThresholds[index];
}

uint8_t AppConfig_SetVehicleAlertThreshold(uint8_t index, uint16_t value)
{
  ConfigMessage message = {0};
  uint8_t applied;

  if (VehicleConfig_IsAlertIndexValid(index) == 0U)
  {
    return 0U;
  }

  message.source = CONFIG_SOURCE_LOCAL;
  message.key = VehicleConfig_AlertIndexToKey(index);
  message.value = value;
  message.originId = 0U;
  message.tick = HAL_GetTick();
  applied = Config_ApplyMessage(&message);
  if (applied != 0U)
  {
    message.value = g_vehicleConfig.alertThresholds[index];
    (void)AppConfig_SendCanConfigU16(message.key, message.value);
  }

  return applied;
}

uint8_t AppConfig_SendCanConfigU8(uint8_t key, uint8_t value)
{
  CanTxMessage message = {0};

  CanTx_FillConfigHeader(&message.header, CAN_CONFIG_RX_ID, FDCAN_DLC_BYTES_2);
  message.length = 2U;
  message.data[0] = key;
  message.data[1] = value;
  CanTx_QueueMessage(&message);
  return 1U;
}

uint8_t AppConfig_SendCanConfigU16(uint8_t key, uint16_t value)
{
  CanTxMessage message = {0};

  CanTx_FillConfigHeader(&message.header, CAN_CONFIG_RX_ID, FDCAN_DLC_BYTES_4);
  message.length = 4U;
  message.data[0] = key;
  message.data[1] = (uint8_t)(value & 0xFFU);
  message.data[2] = (uint8_t)((value >> 8) & 0xFFU);
  message.data[3] = 0U;
  CanTx_QueueMessage(&message);
  return 1U;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  Debug_LogUart5("[RTOS] STACK OVERFLOW\r\n");
  if (pcTaskName != NULL)
  {
    Debug_LogUart5(pcTaskName);
    Debug_LogUart5("\r\n");
  }
  __disable_irq();
  for (;;)
  {
  }
}

void vApplicationMallocFailedHook(void)
{
  Debug_LogUart5("[RTOS] MALLOC FAILED\r\n");
  __disable_irq();
  for (;;)
  {
  }
}

/* USER CODE END Application */

