#include "nrf24.h"

#include "main.h"
#include "spi.h"
#include <string.h>

static HAL_StatusTypeDef NRF24_SpiTransfer(uint8_t value, uint8_t *rxValue);
static HAL_StatusTypeDef NRF24_ExecuteCommand(uint8_t command, const uint8_t *txData, uint8_t *rxData, uint8_t length, uint8_t *statusValue);
static HAL_StatusTypeDef NRF24_SetConfigRegister(uint8_t configValue);
static HAL_StatusTypeDef NRF24_WaitIrqAssert(uint32_t timeoutMs, uint8_t *statusValue);
static void NRF24_Settle(void);

#define NRF24_CMD_R_RX_PAYLOAD          0x61U
#define NRF24_CMD_W_TX_PAYLOAD          0xA0U
#define NRF24_CMD_FLUSH_TX              0xE1U
#define NRF24_CMD_FLUSH_RX              0xE2U

#define NRF24_REG_CONFIG                0x00U
#define NRF24_REG_EN_AA                 0x01U
#define NRF24_REG_EN_RXADDR             0x02U
#define NRF24_REG_SETUP_AW              0x03U
#define NRF24_REG_SETUP_RETR            0x04U
#define NRF24_REG_RF_CH                 0x05U
#define NRF24_REG_RF_SETUP              0x06U
#define NRF24_REG_STATUS                0x07U
#define NRF24_REG_RX_ADDR_P0            0x0AU
#define NRF24_REG_TX_ADDR               0x10U
#define NRF24_REG_RX_PW_P0              0x11U
#define NRF24_REG_FIFO_STATUS           0x17U
#define NRF24_REG_DYNPD                 0x1CU
#define NRF24_REG_FEATURE               0x1DU

#define NRF24_CONFIG_EN_CRC             0x08U
#define NRF24_CONFIG_PWR_UP             0x02U
#define NRF24_CONFIG_PRIM_RX            0x01U

#define NRF24_STATUS_RX_DR              0x40U
#define NRF24_STATUS_TX_DS              0x20U
#define NRF24_STATUS_MAX_RT             0x10U

#define NRF24_FIFO_STATUS_RX_EMPTY      0x01U

static const uint8_t g_nrf24Address[5] = { 'D', 'S', 'P', '0', '1' };

void NRF24_BusInit(void)
{
  NRF24_SetCE(0U);
  NRF24_SetCSN(1U);
  NRF24_Settle();
}

void NRF24_SetCE(uint8_t high)
{
  HAL_GPIO_WritePin(SPI_CE_GPIO_Port, SPI_CE_Pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void NRF24_SetCSN(uint8_t high)
{
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

GPIO_PinState NRF24_GetIrqState(void)
{
  return HAL_GPIO_ReadPin(SPI_IRQ_GPIO_Port, SPI_IRQ_Pin);
}

HAL_StatusTypeDef NRF24_ReadStatusNop(uint8_t *statusValue)
{
  HAL_StatusTypeDef halStatus;
  uint8_t status = 0U;

  if (statusValue == NULL)
  {
    return HAL_ERROR;
  }

  NRF24_SetCSN(0U);
  halStatus = NRF24_SpiTransfer(0xFFU, &status);
  NRF24_SetCSN(1U);

  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  *statusValue = status;
  return HAL_OK;
}

HAL_StatusTypeDef NRF24_ReadRegister(uint8_t reg, uint8_t *value, uint8_t *statusValue)
{
  return NRF24_ReadRegisterMulti(reg, value, 1U, statusValue);
}

HAL_StatusTypeDef NRF24_ReadRegisterMulti(uint8_t reg, uint8_t *values, uint8_t length, uint8_t *statusValue)
{
  HAL_StatusTypeDef halStatus;
  uint8_t status = 0U;

  if ((values == NULL) || (length == 0U))
  {
    return HAL_ERROR;
  }

  halStatus = NRF24_ExecuteCommand((uint8_t)(reg & 0x1FU), NULL, values, length, &status);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  if (statusValue != NULL)
  {
    *statusValue = status;
  }

  return HAL_OK;
}

HAL_StatusTypeDef NRF24_WriteRegister(uint8_t reg, uint8_t value, uint8_t *statusValue)
{
  return NRF24_WriteRegisterMulti(reg, &value, 1U, statusValue);
}

HAL_StatusTypeDef NRF24_WriteRegisterMulti(uint8_t reg, const uint8_t *values, uint8_t length, uint8_t *statusValue)
{
  HAL_StatusTypeDef halStatus;
  uint8_t status = 0U;

  if ((values == NULL) || (length == 0U))
  {
    return HAL_ERROR;
  }

  halStatus = NRF24_ExecuteCommand((uint8_t)(0x20U | (reg & 0x1FU)), values, NULL, length, &status);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  if (statusValue != NULL)
  {
    *statusValue = status;
  }

  return HAL_OK;
}

HAL_StatusTypeDef NRF24_ApplyDefaultConfig(void)
{
  HAL_StatusTypeDef halStatus;
  uint8_t payloadWidth = NRF24_MAX_PAYLOAD_SIZE;

  halStatus = NRF24_WriteRegister(NRF24_REG_EN_AA, 0x01U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_EN_RXADDR, 0x01U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_SETUP_AW, 0x03U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_SETUP_RETR, 0x2FU, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_RF_CH, 0x2AU, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_RF_SETUP, 0x06U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegisterMulti(NRF24_REG_RX_ADDR_P0, g_nrf24Address, sizeof(g_nrf24Address), NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegisterMulti(NRF24_REG_TX_ADDR, g_nrf24Address, sizeof(g_nrf24Address), NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_RX_PW_P0, payloadWidth, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_DYNPD, 0x00U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_WriteRegister(NRF24_REG_FEATURE, 0x00U, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_FlushRx();
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_FlushTx();
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_ClearIrqFlags((uint8_t)(NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT));
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  return NRF24_EnterRxMode();
}

HAL_StatusTypeDef NRF24_RunSelfTest(NRF24_TestResult *result)
{
  HAL_StatusTypeDef halStatus;
  NRF24_TestResult localResult = {0};

  NRF24_BusInit();

  localResult.irqState = NRF24_GetIrqState();

  halStatus = NRF24_ReadStatusNop(&localResult.nopStatus);
  if (halStatus != HAL_OK)
  {
    localResult.halStatus = halStatus;
    goto done;
  }

  halStatus = NRF24_ReadRegister(0x00U, &localResult.config, &localResult.status);
  if (halStatus != HAL_OK)
  {
    localResult.halStatus = halStatus;
    goto done;
  }

  halStatus = NRF24_ReadRegister(0x05U, &localResult.rfChannelBefore, NULL);
  if (halStatus != HAL_OK)
  {
    localResult.halStatus = halStatus;
    goto done;
  }

  halStatus = NRF24_WriteRegister(0x05U, 0x2AU, &localResult.writeStatus);
  if (halStatus != HAL_OK)
  {
    localResult.halStatus = halStatus;
    goto done;
  }

  halStatus = NRF24_ReadRegister(0x05U, &localResult.rfChannelAfter, NULL);
  if (halStatus != HAL_OK)
  {
    localResult.halStatus = halStatus;
    goto done;
  }

  localResult.halStatus = HAL_OK;

done:
  if (result != NULL)
  {
    *result = localResult;
  }
  return localResult.halStatus;
}

static HAL_StatusTypeDef NRF24_SpiTransfer(uint8_t value, uint8_t *rxValue)
{
  uint8_t rx = 0U;
  HAL_StatusTypeDef halStatus = HAL_SPI_TransmitReceive(&hspi1, &value, &rx, 1U, 100U);

  if (rxValue != NULL)
  {
    *rxValue = rx;
  }

  return halStatus;
}

HAL_StatusTypeDef NRF24_ClearIrqFlags(uint8_t irqMask)
{
  return NRF24_WriteRegister(NRF24_REG_STATUS, (uint8_t)(irqMask & (NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT)), NULL);
}

HAL_StatusTypeDef NRF24_FlushRx(void)
{
  return NRF24_ExecuteCommand(NRF24_CMD_FLUSH_RX, NULL, NULL, 0U, NULL);
}

HAL_StatusTypeDef NRF24_FlushTx(void)
{
  return NRF24_ExecuteCommand(NRF24_CMD_FLUSH_TX, NULL, NULL, 0U, NULL);
}

HAL_StatusTypeDef NRF24_EnterRxMode(void)
{
  HAL_StatusTypeDef halStatus = NRF24_SetConfigRegister((uint8_t)(NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX));
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  NRF24_SetCE(1U);
  return HAL_OK;
}

HAL_StatusTypeDef NRF24_SendPayload(const uint8_t *payload, uint8_t length, uint32_t timeoutMs)
{
  HAL_StatusTypeDef halStatus;
  uint8_t txBuffer[NRF24_MAX_PAYLOAD_SIZE] = {0};
  uint8_t status = 0U;

  if ((payload == NULL) || (length == 0U) || (length > NRF24_MAX_PAYLOAD_SIZE))
  {
    return HAL_ERROR;
  }

  (void)memcpy(txBuffer, payload, length);

  NRF24_SetCE(0U);
  halStatus = NRF24_SetConfigRegister((uint8_t)(NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP));
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_ClearIrqFlags((uint8_t)(NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT));
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_FlushTx();
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  halStatus = NRF24_ExecuteCommand(NRF24_CMD_W_TX_PAYLOAD, txBuffer, NULL, NRF24_MAX_PAYLOAD_SIZE, &status);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  NRF24_SetCE(1U);
  for (volatile uint32_t i = 0U; i < 800U; ++i)
  {
    __NOP();
  }
  NRF24_SetCE(0U);

  halStatus = NRF24_WaitIrqAssert(timeoutMs, &status);
  (void)NRF24_ClearIrqFlags((uint8_t)(NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT));
  (void)NRF24_EnterRxMode();

  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  if ((status & NRF24_STATUS_TX_DS) != 0U)
  {
    return HAL_OK;
  }

  if ((status & NRF24_STATUS_MAX_RT) != 0U)
  {
    (void)NRF24_FlushTx();
    return HAL_TIMEOUT;
  }

  return HAL_ERROR;
}

HAL_StatusTypeDef NRF24_ReadRxPayload(uint8_t *payload, uint8_t length)
{
  if ((payload == NULL) || (length == 0U) || (length > NRF24_MAX_PAYLOAD_SIZE))
  {
    return HAL_ERROR;
  }

  return NRF24_ExecuteCommand(NRF24_CMD_R_RX_PAYLOAD, NULL, payload, length, NULL);
}

static HAL_StatusTypeDef NRF24_ExecuteCommand(uint8_t command, const uint8_t *txData, uint8_t *rxData, uint8_t length, uint8_t *statusValue)
{
  HAL_StatusTypeDef halStatus;
  uint8_t status = 0U;
  uint8_t rx = 0U;

  NRF24_SetCSN(0U);
  halStatus = NRF24_SpiTransfer(command, &status);
  if (halStatus == HAL_OK)
  {
    for (uint8_t index = 0U; index < length; index++)
    {
      halStatus = NRF24_SpiTransfer((txData != NULL) ? txData[index] : 0xFFU, &rx);
      if (halStatus != HAL_OK)
      {
        break;
      }

      if (rxData != NULL)
      {
        rxData[index] = rx;
      }
    }
  }
  NRF24_SetCSN(1U);

  if (statusValue != NULL)
  {
    *statusValue = status;
  }

  return halStatus;
}

static HAL_StatusTypeDef NRF24_SetConfigRegister(uint8_t configValue)
{
  HAL_StatusTypeDef halStatus = NRF24_WriteRegister(NRF24_REG_CONFIG, configValue, NULL);
  if (halStatus != HAL_OK)
  {
    return halStatus;
  }

  HAL_Delay(2U);
  return HAL_OK;
}

static HAL_StatusTypeDef NRF24_WaitIrqAssert(uint32_t timeoutMs, uint8_t *statusValue)
{
  uint32_t startTick = HAL_GetTick();
  uint8_t status = 0U;

  do
  {
    if (NRF24_ReadStatusNop(&status) != HAL_OK)
    {
      return HAL_ERROR;
    }

    if ((status & (NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT | NRF24_STATUS_RX_DR)) != 0U)
    {
      if (statusValue != NULL)
      {
        *statusValue = status;
      }

      return HAL_OK;
    }

    if (NRF24_GetIrqState() == GPIO_PIN_RESET)
    {
      /* IRQ should go low when any event bit is set. Read STATUS again so we
         can trust the latched flags even if the IRQ pulse is short or noisy. */
      if (NRF24_ReadStatusNop(&status) != HAL_OK)
      {
        return HAL_ERROR;
      }

      if (statusValue != NULL)
      {
        *statusValue = status;
      }

      if ((status & (NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT | NRF24_STATUS_RX_DR)) != 0U)
      {
        return HAL_OK;
      }
    }
  } while ((HAL_GetTick() - startTick) < timeoutMs);

  if (NRF24_ReadStatusNop(&status) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (statusValue != NULL)
  {
    *statusValue = status;
  }

  if ((status & (NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT | NRF24_STATUS_RX_DR)) == 0U)
  {
    return HAL_TIMEOUT;
  }

  return HAL_OK;
}

static void NRF24_Settle(void)
{
  for (volatile uint32_t i = 0U; i < 200000U; ++i)
  {
    __NOP();
  }
}
