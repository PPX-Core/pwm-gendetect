#ifndef __HAL_PWM_H__
#define __HAL_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool HPWM_Init(void);
void HPWM_Start(void);
void HPWM_Stop(void);
bool HPWM_SetPulse(uint16_t prescaler, uint32_t period, uint32_t pulse);

#ifdef __cplusplus
}
#endif

#endif
