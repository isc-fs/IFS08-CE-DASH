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
#include "cmsis_os.h"
#include "crc.h"
#include "dma2d.h"
#include "fatfs.h"
#include "fdcan.h"
#include "jpeg.h"
#include "ltdc.h"
#include "memorymap.h"
#include "quadspi.h"
#include "sdmmc.h"
#include "spi.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "fmc.h"
#include "app_touchgfx.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

extern uint8_t ltdc_init_ok;
extern uint8_t ltdc_init_fail_step;
extern uint8_t sdmmc1_init_ok;
volatile uint8_t g_sd_preos_done = 0U;
volatile uint8_t g_sd_preos_ok = 0U;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define QSPI_CMD_JEDEC_ID                ((uint8_t)0x9F)
#define QSPI_CMD_READ_DATA               ((uint8_t)0x03)
#define QSPI_CMD_FAST_READ               ((uint8_t)0x0B)
#define QSPI_CMD_WRITE_ENABLE            ((uint8_t)0x06)
#define QSPI_CMD_READ_STATUS_REG         ((uint8_t)0x05)
#define QSPI_CMD_PAGE_PROGRAM            ((uint8_t)0x02)
#define QSPI_CMD_SECTOR_ERASE_4K         ((uint8_t)0x20)
#define QSPI_STATUS_WIP                  ((uint8_t)0x01)
#define QSPI_STATUS_WEL                  ((uint8_t)0x02)
#define MT25QL128_JEDEC_MFR              ((uint8_t)0x20)
#define MT25QL128_JEDEC_TYPE             ((uint8_t)0xBA)
#define MT25QL128_JEDEC_CAP              ((uint8_t)0x18)
#define QSPI_TIMEOUT_MS                  ((uint32_t)2000U)
#define QSPI_SECTOR_SIZE                 ((uint32_t)4096U)
#define QSPI_PAGE_SIZE                   ((uint32_t)256U)

#define LCD_WIDTH                        ((uint32_t)800U)
#define LCD_HEIGHT                       ((uint32_t)480U)
#define LCD_BPP_BYTES                    ((uint32_t)2U)
#define LCD_FB_SIZE_BYTES                (LCD_WIDTH * LCD_HEIGHT * LCD_BPP_BYTES)
#define LCD_FB_ADDRESS                   ((uint32_t)0xC0000000U)
#define LCD_FB_ADDRESS_ALT               (LCD_FB_ADDRESS + LCD_FB_SIZE_BYTES)
#define LCD_FB_MPU_REGION_SIZE           MPU_REGION_SIZE_2MB
#define LTDC_FORCE_DIRECT_RED_FRAME      0U


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t qspi_test_ok = 0U;
uint8_t ltdc_fail_code = 0U;
uint8_t touchgfx_assets_ok = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static uint8_t QSPI_QuickTest(void);
static uint8_t QSPI_WriteEnable(void);
static uint8_t QSPI_WaitReady(uint32_t timeout_ms);
static uint8_t QSPI_Read(uint32_t address, uint8_t *buffer, uint32_t length);
static uint8_t QSPI_PageProgram(uint32_t address, const uint8_t *buffer, uint32_t length);
static uint8_t QSPI_SectorErase(uint32_t address);
static uint8_t QSPI_ProgramBuffer(uint32_t address, const uint8_t *buffer, uint32_t length);
static uint8_t QSPI_EnableMemoryMappedMode(void);
static uint8_t TouchGFX_AssetsPresent(void);
static void TouchGFX_LogAssetProbe(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t qspi_backup_sector[QSPI_SECTOR_SIZE];
static uint8_t qspi_test_pattern[QSPI_PAGE_SIZE];
static uint8_t qspi_verify_page[QSPI_PAGE_SIZE];
extern uint8_t __extflash_start__;
extern uint8_t __extflash_end__;

__attribute__((weak)) void MX_FATFS_Init(void)
{
}

__attribute__((weak)) void MX_TouchGFX_Init(void)
{
}

__attribute__((weak)) void MX_TouchGFX_PreOSInit(void)
{
}

__attribute__((weak)) osStatus_t osKernelInitialize(void)
{
  return osOK;
}

__attribute__((weak)) osStatus_t osKernelStart(void)
{
  return osOK;
}

__attribute__((weak)) void MX_FREERTOS_Init(void)
{
}

static uint8_t QSPI_WriteEnable(void)
{
  QSPI_CommandTypeDef s_command = {0};
  QSPI_AutoPollingTypeDef s_config = {0};

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_WRITE_ENABLE;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_NONE;
  s_command.DummyCycles = 0;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  s_command.Instruction = QSPI_CMD_READ_STATUS_REG;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.NbData = 1;

  s_config.Match = QSPI_STATUS_WEL;
  s_config.Mask = QSPI_STATUS_WEL;
  s_config.MatchMode = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval = 0x10;
  s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t QSPI_WaitReady(uint32_t timeout_ms)
{
  QSPI_CommandTypeDef s_command = {0};
  QSPI_AutoPollingTypeDef s_config = {0};

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_READ_STATUS_REG;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = 1;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match = 0;
  s_config.Mask = QSPI_STATUS_WIP;
  s_config.MatchMode = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval = 0x10;
  s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, timeout_ms) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t QSPI_Read(uint32_t address, uint8_t *buffer, uint32_t length)
{
  QSPI_CommandTypeDef s_command = {0};

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_READ_DATA;
  s_command.AddressMode = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize = QSPI_ADDRESS_24_BITS;
  s_command.Address = address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = length;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_QSPI_Receive(&hqspi, buffer, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t QSPI_PageProgram(uint32_t address, const uint8_t *buffer, uint32_t length)
{
  QSPI_CommandTypeDef s_command = {0};

  if ((length == 0U) || (length > QSPI_PAGE_SIZE))
  {
    return 0U;
  }

  if (QSPI_WriteEnable() == 0U)
  {
    return 0U;
  }

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_PAGE_PROGRAM;
  s_command.AddressMode = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize = QSPI_ADDRESS_24_BITS;
  s_command.Address = address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = length;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)buffer, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  return QSPI_WaitReady(QSPI_TIMEOUT_MS);
}

static uint8_t QSPI_SectorErase(uint32_t address)
{
  QSPI_CommandTypeDef s_command = {0};

  if (QSPI_WriteEnable() == 0U)
  {
    return 0U;
  }

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_SECTOR_ERASE_4K;
  s_command.AddressMode = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize = QSPI_ADDRESS_24_BITS;
  s_command.Address = address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_NONE;
  s_command.DummyCycles = 0;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  return QSPI_WaitReady(8000U);
}

static uint8_t QSPI_ProgramBuffer(uint32_t address, const uint8_t *buffer, uint32_t length)
{
  uint32_t offset = 0U;
  uint32_t chunk = 0U;

  while (offset < length)
  {
    chunk = length - offset;
    if (chunk > QSPI_PAGE_SIZE)
    {
      chunk = QSPI_PAGE_SIZE;
    }

    if (QSPI_PageProgram(address + offset, &buffer[offset], chunk) == 0U)
    {
      return 0U;
    }

    offset += chunk;
  }

  return 1U;
}

static uint8_t QSPI_QuickTest(void)
{
  QSPI_CommandTypeDef s_command = {0};
  uint8_t jedec_id[3] = {0};
  uint32_t flash_size_bytes = 0U;
  uint32_t test_sector_addr = 0U;

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = 0x9FU;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = 3;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, 1000U) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_QSPI_Receive(&hqspi, jedec_id, 1000U) != HAL_OK)
  {
    return 0U;
  }

  if ((jedec_id[0] == 0x00U) || (jedec_id[0] == 0xFFU))
  {
    return 0U;
  }

  if ((jedec_id[1] == 0x00U) && (jedec_id[2] == 0x00U))
  {
    return 0U;
  }

  if ((jedec_id[0] != MT25QL128_JEDEC_MFR) ||
      (jedec_id[1] != MT25QL128_JEDEC_TYPE) ||
      (jedec_id[2] != MT25QL128_JEDEC_CAP))
  {
    return 0U;
  }

  if ((jedec_id[2] < 0x10U) || (jedec_id[2] > 0x25U))
  {
    return 0U;
  }

  flash_size_bytes = (1UL << jedec_id[2]);
  if (flash_size_bytes < (QSPI_SECTOR_SIZE * 4U))
  {
    return 0U;
  }

  test_sector_addr = flash_size_bytes - (QSPI_SECTOR_SIZE * 2U);

  if (QSPI_Read(test_sector_addr, qspi_backup_sector, QSPI_SECTOR_SIZE) == 0U)
  {
    return 0U;
  }

  for (uint32_t i = 0U; i < QSPI_PAGE_SIZE; i++)
  {
    qspi_test_pattern[i] = (uint8_t)(i ^ 0x5AU);
  }

  if (QSPI_SectorErase(test_sector_addr) == 0U)
  {
    return 0U;
  }

  if (QSPI_PageProgram(test_sector_addr, qspi_test_pattern, QSPI_PAGE_SIZE) == 0U)
  {
    return 0U;
  }

  if (QSPI_Read(test_sector_addr, qspi_verify_page, QSPI_PAGE_SIZE) == 0U)
  {
    return 0U;
  }

  if (memcmp(qspi_test_pattern, qspi_verify_page, QSPI_PAGE_SIZE) != 0)
  {
    return 0U;
  }

  if (QSPI_SectorErase(test_sector_addr) == 0U)
  {
    return 0U;
  }

  if (QSPI_ProgramBuffer(test_sector_addr, qspi_backup_sector, QSPI_SECTOR_SIZE) == 0U)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t QSPI_EnableMemoryMappedMode(void)
{
  QSPI_CommandTypeDef s_command = {0};
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_FAST_READ;
  s_command.AddressMode = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize = QSPI_ADDRESS_24_BITS;
  s_command.Address = 0U;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 8;
  s_command.NbData = 0U;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod = 0U;

  if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t TouchGFX_AssetsPresent(void)
{
  volatile const uint8_t *ext = &__extflash_start__;
  uint32_t ext_size = (uint32_t)(&__extflash_end__ - &__extflash_start__);
  uint32_t sample = 0U;
  uint32_t non_ff = 0U;

  if (ext_size == 0U)
  {
    return 0U;
  }

  sample = (ext_size >= 256U) ? 256U : ext_size;

  for (uint32_t i = 0U; i < sample; i++)
  {
    if (ext[i] != 0xFFU)
    {
      non_ff++;
    }
  }

  return (non_ff > 8U) ? 1U : 0U;
}

static void TouchGFX_LogAssetProbe(void)
{
  volatile const uint8_t *ext = &__extflash_start__;
  uint32_t ext_size = (uint32_t)(&__extflash_end__ - &__extflash_start__);
  char buffer[192];

  if (huart5.gState == HAL_UART_STATE_RESET)
  {
    return;
  }

  if (ext_size < 16U)
  {
    (void)snprintf(buffer,
                   sizeof(buffer),
                   "[BOOT] extflash size=%lu bytes (too small for probe)\r\n",
                   (unsigned long)ext_size);
    Debug_LogUart5(buffer);
    return;
  }

  (void)snprintf(buffer,
                 sizeof(buffer),
                 "[BOOT] asset section @%08lX size=%lu sample=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                 (unsigned long)ext,
                 (unsigned long)ext_size,
                 ext[0], ext[1], ext[2], ext[3],
                 ext[4], ext[5], ext[6], ext[7],
                 ext[8], ext[9], ext[10], ext[11],
                 ext[12], ext[13], ext[14], ext[15]);
  Debug_LogUart5(buffer);
}

  /* Disable D-Cache for a clean test — same approach as HW diagnostic */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* ---- Raw LED init via registers (before HAL, before anything) ---- */
  /* LED1=PA0, LED2=PI8, LED3=PC13 */
  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIOIEN;
  __DSB(); __NOP(); __NOP();
  /* PA0 = output */
  GPIOA->MODER = (GPIOA->MODER & ~(3U << (0*2))) | (1U << (0*2));
  /* PC13 = output */
  GPIOC->MODER = (GPIOC->MODER & ~(3U << (13*2))) | (1U << (13*2));
  /* PI8 = output */
  GPIOI->MODER = (GPIOI->MODER & ~(3U << (8*2))) | (1U << (8*2));
  /* All LEDs OFF */
  GPIOA->BSRR = (1U << (0+16));  /* LED1 OFF */
  GPIOI->BSRR = (1U << (8+16));  /* LED2 OFF */
  GPIOC->BSRR = (1U << (13+16)); /* LED3 OFF */
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* LED1 ON = HAL_Init passed */
  GPIOA->BSRR = (1U << 0);
  /* USER CODE END Init */

  /* Configure the system clock */
  Debug_SetStage(1U, NULL);
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  Debug_SetStage(6U, NULL);
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  Debug_SetStage(7U, NULL);
  MX_GPIO_Init();
  Debug_SetStage(8U, "[BOOT] stage 8 GPIO ok\r\n");
  Debug_SetStage(9U, NULL);
  MX_UART5_Init();
  Debug_LogUart5("[BOOT] UART5 online\r\n");
  Debug_SetStage(10U, NULL);
  MX_DMA2D_Init();
  Debug_SetStage(11U, "[BOOT] stage 11 DMA2D ok\r\n");
  MX_FDCAN2_Init();
  Debug_SetStage(13U, "[BOOT] stage 13 FDCAN2 ok\r\n");
  MX_FMC_Init();
  Debug_SetStage(14U, "[BOOT] stage 14 FMC ok\r\n");
  MX_JPEG_Init();
  Debug_SetStage(15U, "[BOOT] stage 15 JPEG ok\r\n");
  MX_LTDC_Init();
  Debug_SetStage(16U, "[BOOT] stage 16 LTDC ok\r\n");
  MX_QUADSPI_Init();
  Debug_SetStage(17U, "[BOOT] stage 17 QSPI ok\r\n");
  qspi_test_ok = QSPI_QuickTest();
  Debug_LogUart5((qspi_test_ok != 0U) ? "[BOOT] QSPI JEDEC quick test ok\r\n" : "[BOOT] QSPI JEDEC quick test fail\r\n");
  MX_SDMMC1_SD_Init();
  Debug_SetStage(18U, "[BOOT] stage 18 SDMMC ok\r\n");
  MX_USART1_Init();
  Debug_SetStage(19U, "[BOOT] stage 19 USART1 ok\r\n");
  Debug_LogUart5((HAL_SDRAM_GetState(&hsdram1) == HAL_SDRAM_STATE_READY) ? "[BOOT] SDRAM ready\r\n" : "[BOOT] SDRAM not ready\r\n");
  Debug_LogUart5((ltdc_init_ok != 0U) ? "[BOOT] LTDC init ok\r\n" : "[BOOT] LTDC init fail\r\n");
  Panel_ClearFramebuffers(0x00U, 0x00U, 0x00U);
  Debug_LogUart5("[BOOT] framebuffer cleared\r\n");
  Panel_ExitStandby();
#if (LTDC_FORCE_DIRECT_RED_FRAME != 0U)
  Panel_ClearFramebuffers(0xFFU, 0x00U, 0x00U);
  Debug_LogUart5("[BOOT] direct red frame mode\r\n");
  while (1)
  {
    HAL_Delay(1000U);
  }
#endif
  MX_USB_OTG_FS_PCD_Init();
  Debug_SetStage(11U, "[BOOT] stage 11 USB ok\r\n");
  MX_CRC_Init();
  Debug_SetStage(12U, "[BOOT] stage 12 CRC ok\r\n");
  MX_SPI1_Init();
  Debug_SetStage(13U, "[BOOT] stage 13 SPI1 ok\r\n");
  MX_FATFS_Init();
  Debug_SetStage(14U, "[BOOT] stage 14 FATFS ok\r\n");
  HAL_Delay(200U);

  /* Call PreOsInit function */
  Debug_LogUart5("[BOOT] TouchGFX PreOS init...\r\n");
  MX_TouchGFX_PreOSInit();
  Debug_SetStage(20U, "[BOOT] stage 20 PreOS ok\r\n");
  touchgfx_assets_ok = QSPI_EnableMemoryMappedMode();
  if (touchgfx_assets_ok != 0U)
  {
    TouchGFX_LogAssetProbe();
    touchgfx_assets_ok = TouchGFX_AssetsPresent();
  }
  Debug_LogUart5((touchgfx_assets_ok != 0U) ? "[BOOT] asset section present\r\n" : "[BOOT] asset section not detected\r\n");
  Debug_LogUart5("[BOOT] TouchGFX init...\r\n");
  MX_TouchGFX_Init();
  Debug_SetStage(21U, "[BOOT] stage 21 TouchGFX init ok\r\n");
  Debug_LogUart5("[BOOT] TouchGFX init returned\r\n");
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  Debug_LogUart5("[BOOT] osKernelInitialize\r\n");
  osKernelInitialize();
  Debug_SetStage(22U, "[BOOT] stage 22 kernel init ok\r\n");

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();
  Debug_SetStage(23U, "[BOOT] stage 23 freertos objects ok\r\n");

  /* Start scheduler */
  Debug_LogUart5("[BOOT] starting scheduler\r\n");
  Debug_SetRuntimeLogsEnabled(0U);
  Debug_SetStage(24U, NULL);
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  Debug_SetStage(2U, NULL);

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_CSI
                              |RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  Debug_SetStage(4U, NULL);
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

  Debug_SetStage(5U, NULL);
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.SubRegionDisable = 0x0;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  char errMsg[48];
  uint8_t blinkCount = (g_ltdc_stage == 0U) ? 9U : g_ltdc_stage;

  (void)snprintf(errMsg, sizeof(errMsg), "[ERR] Error_Handler stage=%u\r\n", g_ltdc_stage);
  Debug_LogUart5(errMsg);

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

  __disable_irq();
  while (1)
  {
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);

    for (uint8_t i = 0U; i < blinkCount; i++)
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
      Debug_SpinDelay(40000000U);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
      Debug_SpinDelay(40000000U);
    }

    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    Debug_SpinDelay(100000000U);
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
