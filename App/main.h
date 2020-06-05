#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STM32F030x6
#define STM32F030x6
#endif
#include "stm32f0xx_hal.h"
#include <stdbool.h>

void pwm_out_init(void);
bool pwm_out_process(uint8_t key);

void pwm_in_init(void);
bool pwm_in_process(uint8_t key);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif
