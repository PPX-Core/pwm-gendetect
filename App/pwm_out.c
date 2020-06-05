#include <stdio.h>
#include "main.h"
#include "hal_pwm.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "hal_led.h"

#define CPU_FREQ    48000000000LL   // 48 MHz
#define MAX_FREQ    12000000000LL   // 12 MHz
#define MIN_FREQ    12LL            // 0.012 Hz
#define INIT_FREQ   100000000LL     // 100 kHz
#define INIT_DUTY   500

#define KEY_UP      KEY1
#define KEY_DOWN    KEY2
#define KEY_LEFT    KEY3
#define KEY_RIGHT   KEY4

static uint64_t freq; // in unit of 0.001 Hz
static uint16_t duty; // in unit of 0.1%
static uint8_t digits[10];
static int8_t cursor, running, setting;
static uint32_t prescaler, period, pulse;

static void pwm_out_freq2digits(void)
{
    uint32_t i, value;
    if (freq >= 1000000000LL)
    {
        value = (freq + 500000LL) / 1000000LL;
        digits[6] = 2;
    }
    else if (freq >= 1000000LL)
    {
        value = (freq + 500LL) / 1000LL;
        digits[6] = 1;
    }
    else
    {
        value = freq;
        digits[6] = 0;
    }
    for (i = 6; i > 0; i--)
    {
        digits[i - 1] = value % 10;
        value /= 10;
    }
}

static void pwm_out_digits2freq(void)
{
    uint32_t i, value = 0;
    for (i = 0; i < 6; i++)
    {
        value *= 10;
        value += digits[i];
    }
    if (digits[6] == 2)
        freq = (uint64_t)value * 1000000LL;
    else if (digits[6] == 1)
        freq = (uint64_t)value * 1000LL;
    else
        freq = (uint64_t)value;
}

static void pwm_out_duty2digits(void)
{
    uint32_t i, value = duty;
    for (i = 9; i >= 7; i--)
    {
        digits[i] = value % 10;
        value /= 10;
    }
}

static void pwm_out_digits2duty(void)
{
    uint32_t i;
    duty = 0;
    for (i = 7; i <= 9; i++)
    {
        duty *= 10;
        duty += digits[i];
    }
}

static void pwm_out_recalculate(void)
{
    uint64_t value;
    if (freq > MAX_FREQ) freq = MAX_FREQ;
    if (freq < MIN_FREQ) freq = MIN_FREQ;
    if (duty == 0) duty = 1;
    prescaler = 1;
    value = (CPU_FREQ + freq/2) / freq;
    while (value > 0x10000LL)
    {
        prescaler <<= 1;
        value >>= 1;
    }
    period = value;
    value *= prescaler;
    freq = CPU_FREQ / value;
    pulse = period * duty / 1000;
    if (pulse == period) pulse--;
    if (pulse == 1) pulse++;
    duty = (pulse * 1000) / period;
}

static void pwm_out_display(void)
{
    char str[17] = "  0.000 Hz  0.0%";
    char unit[3] = {' ', 'k', 'M'};

    if (digits[0]) str[0] = '0' + digits[0];
    if (digits[1] || digits[0]) str[1] = '0' + digits[1];
    str[2] += digits[2];
    str[4] += digits[3];
    str[5] += digits[4];
    str[6] += digits[5];
    str[7] = unit[digits[6]];
    if (digits[7]) str[11] = '0' + digits[7];
    str[12] += digits[8];
    str[14] += digits[9];
    HLCD_PutStr(0, 1, str);
}

static void pwm_out_run(void)
{
    pwm_out_recalculate();
    pwm_out_freq2digits();
    pwm_out_duty2digits();
    pwm_out_display();
    HPWM_SetPulse(prescaler - 1, period - 1, pulse - 1);
    HPWM_Start();
}

static void pwm_out_config(uint8_t key)
{
    int8_t pos[10] = {0, 1, 2, 4, 5, 6, 7, 11, 12, 14};
    switch (key)
    {
    case KEY_RIGHT:
        cursor++;
        break;
    case KEY_LEFT:
        if (cursor > 0) cursor--;
        break;
    case KEY_UP:
        if ((cursor != 6 && digits[cursor] < 9) ||
            (cursor == 6 && digits[cursor] < 2)) digits[cursor]++;
        break;
    case KEY_DOWN:
        if (digits[cursor] > 0) digits[cursor]--;
        break;
    }
    HLCD_Cursor(false);
    pwm_out_display();
    HLCD_SetCursor(pos[cursor], 1);
    HLCD_Cursor(true);
}

void pwm_out_init(void)
{
    HPWM_Init();
    freq = INIT_FREQ;
    duty = INIT_DUTY;
    pwm_out_run();
    running = true;
    setting = false;
}

bool pwm_out_process(uint8_t key)
{
    if (key == KEY_NONE)
    {
        if (running)
        {
            HPWM_Stop();
            HLCD_PutStr(0, 1, "OUTPUT OFF (SW2)");
            running = false;
        }
        else
        {
            pwm_out_run();
            running = true;
        }
        setting = false;
    }
    else if (!setting && running && key == KEY1)
    {
        HPWM_Stop();
        HLCD_SetCursor(0, 1);
        HLCD_Cursor(true);
        HLCD_Blink(true);
        HLED_Off();
        running = false;
        setting = true;
        cursor = 0;
    }
    else if (setting && key == KEY4 && cursor == 9)
    {
        pwm_out_digits2freq();
        pwm_out_digits2duty();
        HLCD_Cursor(false);
        HLCD_Blink(false);
        pwm_out_run();
        running = true;
        setting = false;
    }
    else if (setting)
    {
        pwm_out_config(key);
    }
    return setting;
}
