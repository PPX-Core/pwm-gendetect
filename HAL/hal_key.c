#include <stdio.h>
#include "main.h"
#include "hal_adc.h"
#include "hal_key.h"

#define DMA_SIZE    4
static volatile uint16_t key_buff[DMA_SIZE] __attribute__((aligned(32)));

static HKEY_NUM key_pressed = KEY_NONE;
static uint32_t key_timestamp;

/* ADC measures voltage level on each key with resistors.
 *  1.57V  0.94V  0.55V   ADC
 *                         |
 *  -- 5.1K --- 2K --- 2K -+- 10K --- 3.3V
 *   |       |      |      |
 *  KEY1   KEY2   KEY3    KEY4
 *   |       |      |      |
 *  --------------------------------- GND
 */
const uint16_t key_thresh[4] = {500, 900, 1500, 2200};

bool HKEY_Init(void)
{
    HADC_Init();
    key_buff[0] = 4095;
    key_buff[1] = 4095;
    key_buff[2] = 4095;
    key_buff[3] = 4095;
    HADC_Start((uint32_t *)key_buff, DMA_SIZE);
    return true;
}

int HKEY_Scan(void)
{
    HKEY_NUM key;
    uint16_t i, level = key_buff[0];
    for (i = 1; i < DMA_SIZE; i++)
    {
        uint16_t diff;
        if (level >= key_buff[i])
            diff = level - key_buff[i];
        else
            diff = key_buff[i] - level;
        if (diff > 300) return key_pressed;
    }
    if (level > key_thresh[3])
        key = KEY_NONE;
    else if (level > key_thresh[2])
        key = KEY1;
    else if (level > key_thresh[1])
        key = KEY2;
    else if (level > key_thresh[0])
        key = KEY3;
    else
        key = KEY4;
    if (key != key_pressed)
    {
        key_pressed = key;
        key_timestamp = HAL_GetTick();
    }
    return key_pressed;
}

int HKEY_GetPeriod(void)
{
    return (HAL_GetTick() - key_timestamp);
}
