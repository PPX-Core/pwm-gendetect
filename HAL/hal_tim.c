#include "main.h"
#include "hal_tim.h"

static TIM_HandleTypeDef htim16;

bool HTIM_Init(void)
{
    __HAL_RCC_TIM16_CLK_ENABLE();
    htim16.Instance = TIM16;
    htim16.Init.Prescaler = 47;
    htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim16.Init.Period = 0xffff;
    htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim16.Init.RepetitionCounter = 0;
    htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
    {
        Error_Handler();
    }
    return true;
}

void HTIM_Start(void)
{
    HAL_TIM_Base_Stop(&htim16);
    __HAL_TIM_SET_COUNTER(&htim16,0);
    HAL_TIM_Base_Start(&htim16);
}

uint32_t HTIM_Stop(void)
{
    HAL_TIM_Base_Stop(&htim16);
    return __HAL_TIM_GET_COUNTER(&htim16);
}

uint32_t HTIM_GetCount(void)
{
    return __HAL_TIM_GET_COUNTER(&htim16);
}

void HTIM_DelayUS(uint32_t us)
{
    HAL_TIM_Base_Stop(&htim16);
    __HAL_TIM_SET_COUNTER(&htim16, 0);
    HAL_TIM_Base_Start(&htim16);
    while (__HAL_TIM_GET_COUNTER(&htim16) < us);
    HAL_TIM_Base_Stop(&htim16);
}

void HTIM_DelayMS(uint32_t ms)
{
    while (ms-- > 0) HTIM_DelayUS(1000);
}
