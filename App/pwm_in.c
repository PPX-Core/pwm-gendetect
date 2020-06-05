#include <stdio.h>
#include "main.h"
#include "hal_ic.h"
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#define CPU_FREQ    48000000000LL
#define STOPPED     0
#define PRESET      1
#define RUNNING     2

static uint16_t state;
static uint16_t prev_period;

static void pwm_in_display(uint64_t freq, uint32_t duty)
{
    char str[17] = "  0.000 Hz  0.0%";
    uint8_t f[6], d[3];
    uint32_t i, value;
    if (freq >= 1000000000LL)
    {
        value = (freq + 500000LL) / 1000000LL;
        str[7] = 'M';
    }
    else if (freq >= 1000000LL)
    {
        value = (freq + 500LL) / 1000LL;
        str[7] = 'k';
    }
    else
    {
        value = freq;
        str[7] = ' ';
    }
    for (i = 6; i > 0; i--)
    {
        f[i - 1] = value % 10;
        value /= 10;
    }
    for (i = 3; i > 0; i--)
    {
        d[i - 1] = duty % 10;
        duty /= 10;
    }
    if (f[0]) str[0] = '0' + f[0];
    if (f[1] || f[0]) str[1] = '0' + f[1];
    str[2] = '0' + f[2];
    str[4] = '0' + f[3];
    str[5] = '0' + f[4];
    str[6] = '0' + f[5];
    if (d[0]) str[11] = '0' + d[0];
    str[12] = '0' + d[1];
    str[14] = '0' + d[2];
    HLCD_PutStr(0, 0, str);
}

#define ABS(a, b)   ((a) >= (b)) ? ((a) - (b)) : ((b) - (a))
static bool pwm_in_sampling(void)
{
    static uint32_t timeticks;
    uint16_t period, width, diff;
    uint32_t prescaler;
    uint64_t freq;
    uint32_t duty, value;

    if (!HIC_Detect(&period, &width, &prescaler))
    {
        if ((HAL_GetTick() - timeticks) > 10000)
        {
            state = PRESET;
            HLCD_PutStr(0, 0, "RETRY ...       ");
        }
        HLED_Off();
        return false;
    }

    value = prescaler * period;
    freq = (CPU_FREQ + (uint64_t)value / 2) / value;
    duty = ((uint32_t)width * 1000 + period / 2) / period;
    diff = ABS(period, prev_period);
    if ((prev_period != 0 && diff >= (period / 4)) ||
        period < 5 || duty == 0 || duty > 990)
    {
        state = PRESET;
        HLCD_PutStr(0, 0, "RESET ...       ");
        HLED_Off();
        return false;
    }

    if (prev_period == 0) prev_period = period;
    HLED_On();
    timeticks = HAL_GetTick();
    pwm_in_display(freq, duty);
    return true;
}

void pwm_in_init(void)
{
    HIC_Init();
    HIC_Preset();
    state = PRESET;
}

bool pwm_in_process(uint8_t key)
{
    if (key != KEY_NONE)
    {
        if (state != STOPPED)
        {
            HLED_Off();
            HIC_Stop();
            HLCD_PutStr(0, 0, "SAMPLE OFF (SW3)");
            state = STOPPED;
        }
        else
        {
            HLCD_PutStr(0, 0, "                ");
            state = PRESET;
        }
    }
    if (state == PRESET)
    {
        if (HIC_Preset())
        {
            state = RUNNING;
            prev_period = 0;
        }
    }
    else if (state == RUNNING)
    {
        return pwm_in_sampling();
    }
    return false;
}
