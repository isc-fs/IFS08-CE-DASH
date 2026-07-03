#include "stm32h7xx_hal.h"
#include "Dev_Inf.h"

QSPI_HandleTypeDef hqspi;

#define MT25QL128_JEDEC_MFR         ((uint8_t)0x20U)
#define MT25QL128_JEDEC_TYPE        ((uint8_t)0xBAU)
#define MT25QL128_JEDEC_CAP         ((uint8_t)0x18U)

#define QSPI_CMD_JEDEC_ID           ((uint8_t)0x9FU)
#define QSPI_CMD_READ_DATA          ((uint8_t)0x03U)
#define QSPI_CMD_FAST_READ          ((uint8_t)0x0BU)
#define QSPI_CMD_WRITE_ENABLE       ((uint8_t)0x06U)
#define QSPI_CMD_READ_STATUS_REG    ((uint8_t)0x05U)
#define QSPI_CMD_PAGE_PROGRAM       ((uint8_t)0x02U)
#define QSPI_CMD_BLOCK_ERASE_64K    ((uint8_t)0xD8U)
#define QSPI_CMD_BULK_ERASE         ((uint8_t)0xC7U)

#define QSPI_STATUS_WIP             ((uint8_t)0x01U)
#define QSPI_STATUS_WEL             ((uint8_t)0x02U)

#define QSPI_TIMEOUT_MS             ((uint32_t)2000U)
#define QSPI_ERASE_TIMEOUT_MS       ((uint32_t)8000U)
#define QSPI_BULK_TIMEOUT_MS        ((uint32_t)120000U)
#define QSPI_PAGE_SIZE              ((uint32_t)256U)

static void Loader_Delay(volatile uint32_t count)
{
  while (count-- != 0U)
  {
    __NOP();
  }
}

uint32_t HAL_GetTick(void)
{
  static uint32_t tick = 0;
  return ++tick;
}

void HAL_Delay(uint32_t Delay)
{
  while (Delay-- != 0U)
  {
    Loader_Delay(4000U);
  }
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  (void)TickPriority;
  return HAL_OK;
}

void HAL_MspInit(void)
{
}

void HAL_MspDeInit(void)
{
}

void Error_Handler(void)
{
  while (1)
  {
  }
}

static void QSPI_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
  PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  __HAL_RCC_QSPI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_10;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void QSPI_MspDeInit(void)
{
  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2 | GPIO_PIN_10);
  __HAL_RCC_QSPI_CLK_DISABLE();
}

static int QSPI_InitFlash(void)
{
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 3;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 23;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  return (HAL_QSPI_Init(&hqspi) == HAL_OK) ? 1 : 0;
}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef *qspiHandle)
{
  if (qspiHandle->Instance == QUADSPI)
  {
    QSPI_MspInit();
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *qspiHandle)
{
  if (qspiHandle->Instance == QUADSPI)
  {
    QSPI_MspDeInit();
  }
}

static int QSPI_WriteEnable(void)
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
    return 0;
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

  return (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, QSPI_TIMEOUT_MS) == HAL_OK) ? 1 : 0;
}

static int QSPI_WaitReady(uint32_t timeout_ms)
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

  return (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, timeout_ms) == HAL_OK) ? 1 : 0;
}

static int QSPI_ReadData(uint32_t address, uint8_t *buffer, uint32_t length)
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
    return 0;
  }

  return (HAL_QSPI_Receive(&hqspi, buffer, QSPI_TIMEOUT_MS) == HAL_OK) ? 1 : 0;
}

static int QSPI_PageProgram(uint32_t address, const uint8_t *buffer, uint32_t length)
{
  QSPI_CommandTypeDef s_command = {0};

  if ((length == 0U) || (length > QSPI_PAGE_SIZE))
  {
    return 0;
  }

  if (!QSPI_WriteEnable())
  {
    return 0;
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
    return 0;
  }

  if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)buffer, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  return QSPI_WaitReady(QSPI_TIMEOUT_MS);
}

static int QSPI_BlockErase64K(uint32_t address)
{
  QSPI_CommandTypeDef s_command = {0};

  if (!QSPI_WriteEnable())
  {
    return 0;
  }

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_BLOCK_ERASE_64K;
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
    return 0;
  }

  return QSPI_WaitReady(QSPI_ERASE_TIMEOUT_MS);
}

static int QSPI_BulkErase(void)
{
  QSPI_CommandTypeDef s_command = {0};

  if (!QSPI_WriteEnable())
  {
    return 0;
  }

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_BULK_ERASE;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_NONE;
  s_command.DummyCycles = 0;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  return QSPI_WaitReady(QSPI_BULK_TIMEOUT_MS);
}

static int QSPI_CheckJedec(void)
{
  QSPI_CommandTypeDef s_command = {0};
  uint8_t jedec_id[3] = {0};

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction = QSPI_CMD_JEDEC_ID;
  s_command.AddressMode = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode = QSPI_DATA_1_LINE;
  s_command.DummyCycles = 0;
  s_command.NbData = 3;
  s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &s_command, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  if (HAL_QSPI_Receive(&hqspi, jedec_id, QSPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  return (jedec_id[0] == MT25QL128_JEDEC_MFR &&
          jedec_id[1] == MT25QL128_JEDEC_TYPE &&
          jedec_id[2] == MT25QL128_JEDEC_CAP) ? 1 : 0;
}

static int QSPI_ProgramRange(uint32_t address, uint32_t size, uint8_t *buffer)
{
  uint32_t offset = 0U;
  uint32_t chunk = 0U;

  while (offset < size)
  {
    chunk = size - offset;
    if (chunk > QSPI_PAGE_SIZE)
    {
      chunk = QSPI_PAGE_SIZE;
    }

    if (!QSPI_PageProgram(address + offset, &buffer[offset], chunk))
    {
      return 0;
    }

    offset += chunk;
  }

  return 1;
}

int Init(void)
{
  SystemInit();
  HAL_Init();

  if (!QSPI_InitFlash())
  {
    return 0;
  }

  return QSPI_CheckJedec();
}

int DeInit(void)
{
  HAL_QSPI_DeInit(&hqspi);
  return 1;
}

int Read(uint32_t Address, uint32_t Size, uint8_t *Buffer)
{
  return QSPI_ReadData(Address & 0x00FFFFFFU, Buffer, Size);
}

int Write(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
  return QSPI_ProgramRange(Address & 0x00FFFFFFU, Size, buffer);
}

int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
  uint32_t addr = EraseStartAddress & 0x00FF0000U;
  uint32_t end = EraseEndAddress & 0x00FFFFFFU;

  while (addr <= end)
  {
    if (!QSPI_BlockErase64K(addr))
    {
      return 0;
    }
    addr += 0x10000U;
  }

  return 1;
}

int MassErase(void)
{
  return QSPI_BulkErase();
}
