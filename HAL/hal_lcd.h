#ifndef __HAL_LCD_H__
#define __HAL_LCD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool HLCD_Init(void);
void HLCD_Clear(void);
void HLCD_ResetCursor(void);
void HLCD_SetCursor(uint8_t col, uint8_t row);
void HLCD_PutChar(uint8_t value);
void HLCD_PutStr(uint8_t col, uint8_t row, const char *str);
void HLCD_Display(bool enable);
void HLCD_Cursor(bool enable);
void HLCD_Blink(bool enable);

#ifdef __cplusplus
}
#endif

#endif
