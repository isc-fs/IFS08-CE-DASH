#ifndef __UI_BUTTONS_H__
#define __UI_BUTTONS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
  UI_BUTTON_ID_UP = 0,
  UI_BUTTON_ID_DOWN,
  UI_BUTTON_ID_MENU,
  UI_BUTTON_ID_SELECT,
  UI_BUTTON_ID_COUNT
} UiButtonId;

#define UI_BUTTON_MASK_UP       (1UL << UI_BUTTON_ID_UP)
#define UI_BUTTON_MASK_DOWN     (1UL << UI_BUTTON_ID_DOWN)
#define UI_BUTTON_MASK_MENU     (1UL << UI_BUTTON_ID_MENU)
#define UI_BUTTON_MASK_SELECT   (1UL << UI_BUTTON_ID_SELECT)

void UIButtons_TaskInit(void);
void UIButtons_TaskPoll(void);
uint32_t UIButtons_FetchPressedMask(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_BUTTONS_H__ */
