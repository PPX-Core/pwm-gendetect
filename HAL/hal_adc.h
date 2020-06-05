#ifndef __HAL_ADC_H__
#define __HAL_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

bool HADC_Init(void);
bool HADC_Start(uint32_t* data, uint32_t size);
bool HADC_Stop(void);

#ifdef __cplusplus
}
#endif

#endif
