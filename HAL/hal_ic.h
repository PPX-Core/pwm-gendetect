#ifndef __HAL_IC_H__
#define __HAL_IC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/* Init Timer Input Capture */
bool HIC_Init(void);
void HIC_Stop(void);

bool HIC_Preset(void);
bool HIC_Detect(uint16_t *period, uint16_t *width, uint32_t *prescaler);

#ifdef __cplusplus
}
#endif

#endif
