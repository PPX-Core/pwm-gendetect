#ifndef __HAL_TIM_H__
#define __HAL_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool HTIM_Init(void);
void HTIM_Start(void);
uint32_t HTIM_Stop(void);
uint32_t HTIM_GetCount(void);
void HTIM_DelayUS(uint32_t us);
void HTIM_DelayMS(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif
