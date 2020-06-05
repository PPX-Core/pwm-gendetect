#ifndef __HAL_LED_H__
#define __HAL_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "main.h"

#define LED_Pin     GPIO_PIN_4
#define LED_Port    GPIOA

bool HLED_Init(void);

#define HLED_On()       HAL_GPIO_WritePin(LED_Port, LED_Pin, GPIO_PIN_RESET)
#define HLED_Off()      HAL_GPIO_WritePin(LED_Port, LED_Pin, GPIO_PIN_SET)
#define HLED_Toggle()   HAL_GPIO_TogglePin(LED_Port, LED_Pin)

#ifdef __cplusplus
}
#endif

#endif
