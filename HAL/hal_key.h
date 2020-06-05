#ifndef __HAL_KEY_H__
#define __HAL_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    KEY_NONE,
    KEY1,
    KEY2,
    KEY3,
    KEY4
} HKEY_NUM;

bool HKEY_Init(void);
int HKEY_Scan(void);
int HKEY_GetPeriod(void);

#ifdef __cplusplus
}
#endif

#endif
