#include <stdio.h>
#include "main.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_tim.h"
#include "hal_lcd.h"

static void SystemClock_Config(void);

int main(void)
{
    uint32_t timeticks = 0;
    uint8_t prev_key = KEY_NONE;
    uint8_t setting = 0;

    HAL_Init();
    SystemClock_Config();

    HLED_Init();
    HKEY_Init();
    HTIM_Init();
    HLCD_Init();
    HLCD_Clear();

    HLCD_PutStr(0, 0, "PWM Gen/Detector");
    HLCD_PutStr(0, 1, "Release Ver 1.00");
    HAL_Delay(2000);
    HLCD_Clear();

    pwm_out_init();
    pwm_in_init();
    while (1)
    {
        uint8_t key = HKEY_Scan();
        if (key != KEY_NONE)
        {
            /* configuring PWM output */
            if (setting)
            {
                if (key != prev_key || HKEY_GetPeriod() > 500)
                {
                    setting = pwm_out_process(key);
                }
            }
            /* released and pressed */
            else if (prev_key == KEY_NONE)
            {
                switch (key)
                {
                case KEY1:
                    /* start configuration */
                    setting = pwm_out_process(key);
                    break;
                case KEY2:
                    /* turn on/off PWM output */
                    setting = pwm_out_process(KEY_NONE);
                    break;
                case KEY3:
                    /* turn on/off PWM measurement */
                    pwm_in_process(key);
                    break;
                case KEY4:
                    /* do nothing to avoid re-enter of setting */
                    break;
                }
            }
        }
        /* update PWM measurement every a period */
        if (!setting && (HAL_GetTick() - timeticks) > 500)
        {
            if (pwm_in_process(KEY_NONE)) timeticks = HAL_GetTick();
        }
        prev_key = key;
        HAL_Delay(20);
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI14CalibrationValue = 16;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
}

void Error_Handler(void)
{
    while (1)
    {
        HLED_Toggle();
        HAL_Delay(100);
    }
}
