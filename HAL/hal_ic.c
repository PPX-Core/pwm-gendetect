#include <stdio.h>
#include "main.h"
#include "hal_ic.h"

static TIM_HandleTypeDef htim3;
static DMA_HandleTypeDef htim3_dma;

#define MCU_FREQ        48000000L
#define DMA_BUFSIZE     64
static volatile uint16_t dma_buff[DMA_BUFSIZE] __attribute__((aligned(32)));
static volatile bool dma_ready;
static volatile uint16_t cnt_overflow, tim_overflow;
static uint16_t dma_size;
static uint32_t tim_prescaler;

/* Init Timer Input Capture */
bool HIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    dma_ready = false;

    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM3 GPIO Configuration    
        PA6     ------> TIM3_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim3_dma.Instance = DMA1_Channel4;
    htim3_dma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    htim3_dma.Init.PeriphInc = DMA_PINC_DISABLE;
    htim3_dma.Init.MemInc = DMA_MINC_ENABLE;
    htim3_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    htim3_dma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    htim3_dma.Init.Mode = DMA_CIRCULAR;
    htim3_dma.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&htim3_dma) != HAL_OK)
    {
        Error_Handler();
    }

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = tim_prescaler - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_CC1], htim3_dma);
    __HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_TRIGGER], htim3_dma);
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
    return true;
}

void DMA1_Channel4_5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&htim3_dma);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        HAL_TIM_Base_Stop(&htim3);
        tim_overflow = cnt_overflow;
        cnt_overflow = 0;
        dma_ready = true;
    }
}

void TIM3_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
    {
        __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
        cnt_overflow++;
    }
}

static void HIC_Start(uint32_t prescaler, uint16_t size, bool update)
{
    if (update) __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
    __HAL_TIM_SET_PRESCALER(&htim3, prescaler - 1);
    HAL_TIM_GenerateEvent(&htim3, TIM_EVENTSOURCE_UPDATE);
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    dma_size = size;
    dma_ready = false;
    tim_overflow = 0;
    cnt_overflow = 0;
    tim_prescaler = prescaler;

    HAL_TIM_DMABurst_MultiReadStart(&htim3, TIM_DMABASE_CCR1, TIM_DMA_CC1, (uint32_t *)dma_buff, TIM_DMABURSTLENGTH_2TRANSFERS, size);
    TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);
    TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
    HAL_TIM_Base_Start(&htim3);
}

void HIC_Stop(void)
{
    HAL_TIM_Base_Stop(&htim3);
    HAL_TIM_DMABurst_ReadStop(&htim3, TIM_DMA_CC1);
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
    dma_ready = false;
}

bool HIC_Preset(void)
{
    static uint32_t start_ticks = 0;
    static bool running = false;
    if (!running)
    {
        HIC_Stop();
        HIC_Start(1, DMA_BUFSIZE, true);
        running = true;
        start_ticks = HAL_GetTick();
    }
    else
    {
        uint16_t period, width;
        if (dma_ready)
        {
            uint32_t freq;
            running = false;
            HIC_Stop();
            HIC_Detect(&period, &width, NULL);
            tim_overflow /= (dma_size/2 - 1);
            freq = MCU_FREQ / (tim_prescaler * (period + (tim_overflow<<16)));
            if (freq >= 100000L)
                tim_prescaler = 1;
            else if (freq >= 10000L)
                tim_prescaler = 2;
            else if (freq >= 1000L)
                tim_prescaler = 24;
            else if (freq >= 100L)
                tim_prescaler = 240;
            else if (freq >= 10L)
                tim_prescaler = 2400;
            else
                tim_prescaler = 24000;
            HIC_Start(tim_prescaler, 16, false);
            return true;
        }
        else if ((HAL_GetTick() - start_ticks) > 1000)
        {
            running = false;
            HIC_Stop();
            tim_prescaler = 24000L;
            HIC_Start(tim_prescaler, 16, false);
            return true;
        }
    }
    return false;
}

#define ABS(a, b)   ((a) >= (b)) ? ((a) - (b)) : ((b) - (a))
bool HIC_Detect(uint16_t *p, uint16_t *w, uint32_t *d)
{
    if (dma_ready)
    {
        uint16_t offset = (dma_size - __HAL_DMA_GET_COUNTER(&htim3_dma)) & (dma_size - 1);
        uint16_t index1 = (offset - 4) & (dma_size - 1);
        uint16_t index2 = (index1 + 2) & (dma_size - 1);
        uint16_t period = dma_buff[index2] - dma_buff[index1];
        uint16_t width1 = period + dma_buff[index1 + 1] - dma_buff[index1];
        uint16_t width2 = period + dma_buff[index2 + 1] - dma_buff[index2];
        uint16_t diff = ABS(width1, width2);
        if (p) *p = period;
        if (w) *w = width1;
        if (d) *d = tim_prescaler;
        return (period > width1 && period > width2 && diff < (period / 4));
    }
    return false;
}
