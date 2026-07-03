#include "ui_buttons.h"
#include "main.h"

#define UI_BUTTON_ACTIVE_STATE       GPIO_PIN_SET
#define UI_BUTTON_DEBOUNCE_POLLS     4U

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  uint32_t mask;
} UiButtonConfig;

static const UiButtonConfig g_uiButtonConfigs[UI_BUTTON_ID_COUNT] =
{
  {UP_SW_GPIO_Port, UP_SW_Pin, UI_BUTTON_MASK_UP},
  {DOWN_SW_GPIO_Port, DOWN_SW_Pin, UI_BUTTON_MASK_DOWN},
  {MENU_SW_GPIO_Port, MENU_SW_Pin, UI_BUTTON_MASK_MENU},
  {SELECT_SW_GPIO_Port, SELECT_SW_Pin, UI_BUTTON_MASK_SELECT}
};

static uint8_t g_uiButtonsLastRawState[UI_BUTTON_ID_COUNT];
static uint8_t g_uiButtonsStableState[UI_BUTTON_ID_COUNT];
static uint8_t g_uiButtonsDebounceCount[UI_BUTTON_ID_COUNT];
static volatile uint32_t g_uiButtonsPressedMask = 0U;

static uint8_t UIButtons_ReadRaw(UiButtonId buttonId)
{
  GPIO_PinState pinState = GPIO_PIN_RESET;

  if (buttonId >= UI_BUTTON_ID_COUNT)
  {
    return 0U;
  }

  pinState = HAL_GPIO_ReadPin(g_uiButtonConfigs[buttonId].port, g_uiButtonConfigs[buttonId].pin);
  return (pinState == UI_BUTTON_ACTIVE_STATE) ? 1U : 0U;
}

void UIButtons_TaskInit(void)
{
  for (uint32_t index = 0U; index < UI_BUTTON_ID_COUNT; index++)
  {
    uint8_t rawState = UIButtons_ReadRaw((UiButtonId)index);
    g_uiButtonsLastRawState[index] = rawState;
    g_uiButtonsStableState[index] = rawState;
    g_uiButtonsDebounceCount[index] = UI_BUTTON_DEBOUNCE_POLLS;
  }

  g_uiButtonsPressedMask = 0U;
}

void UIButtons_TaskPoll(void)
{
  for (uint32_t index = 0U; index < UI_BUTTON_ID_COUNT; index++)
  {
    uint8_t rawState = UIButtons_ReadRaw((UiButtonId)index);

    if (rawState == g_uiButtonsLastRawState[index])
    {
      if (g_uiButtonsDebounceCount[index] < UI_BUTTON_DEBOUNCE_POLLS)
      {
        g_uiButtonsDebounceCount[index]++;
      }
    }
    else
    {
      g_uiButtonsLastRawState[index] = rawState;
      g_uiButtonsDebounceCount[index] = 1U;
    }

    if ((g_uiButtonsDebounceCount[index] >= UI_BUTTON_DEBOUNCE_POLLS) &&
        (g_uiButtonsStableState[index] != rawState))
    {
      g_uiButtonsStableState[index] = rawState;

      if (rawState != 0U)
      {
        g_uiButtonsPressedMask |= g_uiButtonConfigs[index].mask;
      }
    }
  }
}

uint32_t UIButtons_FetchPressedMask(void)
{
  uint32_t primask = __get_PRIMASK();
  uint32_t pressedMask = 0U;

  __disable_irq();
  pressedMask = g_uiButtonsPressedMask;
  g_uiButtonsPressedMask = 0U;

  if (primask == 0U)
  {
    __enable_irq();
  }

  return pressedMask;
}
