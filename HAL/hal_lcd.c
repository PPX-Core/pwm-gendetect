#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "hal_lcd.h"
#include "hal_tim.h"

// commands
#define LCD1602_CLEARDISPLAY        0x01
#define LCD1602_RETURNHOME          0x02
#define LCD1602_ENTRYMODESET        0x04
#define LCD1602_DISPLAYCONTROL      0x08
#define LCD1602_CURSORSHIFT         0x10
#define LCD1602_FUNCTIONSET         0x20
#define LCD1602_SETCGRAMADDR        0x40
#define LCD1602_SETDDRAMADDR        0x80

// flags for display entry mode
#define LCD1602_ENTRYRIGHT          0x00
#define LCD1602_ENTRYLEFT           0x02
#define LCD1602_ENTRYSHIFTINCREMENT 0x01
#define LCD1602_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD1602_DISPLAYON           0x04
#define LCD1602_DISPLAYOFF          0x00
#define LCD1602_CURSORON            0x02
#define LCD1602_CURSOROFF           0x00
#define LCD1602_BLINKON             0x01
#define LCD1602_BLINKOFF            0x00

// flags for display/cursor shift
#define LCD1602_DISPLAYMOVE         0x08
#define LCD1602_CURSORMOVE          0x00
#define LCD1602_MOVERIGHT           0x04
#define LCD1602_MOVELEFT            0x00

// flags for function set
#define LCD1602_8BITMODE            0x10
#define LCD1602_4BITMODE            0x00
#define LCD1602_2LINE               0x08
#define LCD1602_1LINE               0x00
#define LCD1602_5x10DOTS            0x04
#define LCD1602_5x8DOTS             0x00

#define LCD1602_MAXCOL  40
#define DELAY_US(us)    HTIM_DelayUS(us)
#define DELAY_MS(ms)    HTIM_DelayMS(ms)

#define LCD1602_D7  GPIO_PIN_3
#define LCD1602_D6  GPIO_PIN_2
#define LCD1602_D5  GPIO_PIN_1
#define LCD1602_D4  GPIO_PIN_0
#define LCD1602_EN  GPIO_PIN_13     /* ENABLE or SIGNAL */
#define LCD1602_RS  GPIO_PIN_14     /* DATA or CMD */

#define HIGH            GPIO_PIN_SET
#define LOW             GPIO_PIN_RESET
#define WRINT_PIN(p, v) HAL_GPIO_WritePin(GPIOA, LCD1602_ ## p, (v))

static uint8_t lcd1602_mode;
static uint8_t lcd1602_ctrl;

static inline void lcd1602_pulse(void)
{
    WRINT_PIN(EN, LOW);
    DELAY_US(1);
    WRINT_PIN(EN, HIGH);
    DELAY_US(1);   /* enable pulse must be >450ns */
    WRINT_PIN(EN, LOW);
    DELAY_US(50); /* commands need > 37us to settle */
}

static inline void lcd1602_write4bits(uint8_t value)
{
    WRINT_PIN(D7, (value & 0x08) ? HIGH : LOW);
    WRINT_PIN(D6, (value & 0x04) ? HIGH : LOW);
    WRINT_PIN(D5, (value & 0x02) ? HIGH : LOW);
    WRINT_PIN(D4, (value & 0x01) ? HIGH : LOW);
    lcd1602_pulse();
}

static inline void lcd1602_write(uint8_t value, uint8_t mode)
{
    WRINT_PIN(RS, mode ? HIGH : LOW);
    lcd1602_write4bits(value >> 4);
    lcd1602_write4bits(value);
}

static inline void lcd1602_command(uint8_t value)
{
    lcd1602_write(value, LOW);
}

static void lcd1602_begin(void)
{
    uint8_t lcd1602_func = LCD1602_2LINE | LCD1602_5x8DOTS;

    /* according to datasheet, it needs at least 40ms after power rises above 2.7V
     * before sending commands.
     */
    DELAY_MS(50);

    WRINT_PIN(RS, LOW);
    WRINT_PIN(EN, LOW);
    /* it starts in 8bit mode, try to set 4 bit mode */
    lcd1602_write4bits(0x03);
    DELAY_MS(5); /* wait more than 4.1ms */
    lcd1602_write4bits(0x03);
    DELAY_MS(5); /* wait more than 4.1ms */
    lcd1602_write4bits(0x03);
    DELAY_US(60);
    lcd1602_write4bits(0x02);

    lcd1602_command(LCD1602_FUNCTIONSET | lcd1602_func);

    /* turn the display on with no cursor or blinking default */
    lcd1602_ctrl = LCD1602_DISPLAYON | LCD1602_CURSOROFF | LCD1602_BLINKOFF;  
    lcd1602_command(LCD1602_DISPLAYCONTROL | lcd1602_ctrl);

    /* clear the display off */
    lcd1602_command(LCD1602_CLEARDISPLAY);
    DELAY_MS(2);

    /* set default text direction */
    lcd1602_mode = LCD1602_ENTRYLEFT | LCD1602_ENTRYSHIFTDECREMENT;
    lcd1602_command(LCD1602_ENTRYMODESET | lcd1602_mode);
}

bool HLCD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, LCD1602_D4|LCD1602_D5|LCD1602_D6|LCD1602_D7
                            |LCD1602_EN|LCD1602_RS, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LCD1602_D4|LCD1602_D5|LCD1602_D6|LCD1602_D7
                         |LCD1602_EN|LCD1602_RS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    lcd1602_begin();
    return true;
}

/* clear display and set cursor position to zero */
void HLCD_Clear(void)
{
    lcd1602_command(LCD1602_CLEARDISPLAY);
    DELAY_MS(2);
}

/* set cursor position to zero */
void HLCD_ResetCursor(void)
{
    lcd1602_command(LCD1602_RETURNHOME);
    DELAY_MS(2);
}

/* set cursor position */
void HLCD_SetCursor(uint8_t col, uint8_t row)
{
    uint8_t pos = row ? (0x40 + col): col;
    lcd1602_command(LCD1602_SETDDRAMADDR | pos);
}

/* write character */
void HLCD_PutChar(uint8_t value)
{
    lcd1602_write(value, HIGH);
}

void HLCD_PutStr(uint8_t col, uint8_t row, const char *str)
{
    HLCD_SetCursor(col, row);
    while ((*str) != '\0')
    {
        lcd1602_write(*str++, HIGH);
    }
}

/* turn on/off LCD display */
void HLCD_Display(bool enable)
{
    if (enable)
        lcd1602_ctrl |= LCD1602_DISPLAYON;
    else
        lcd1602_ctrl &= ~LCD1602_DISPLAYON;
    lcd1602_command(LCD1602_DISPLAYCONTROL | lcd1602_ctrl);
}

/* turn on/off cursor display */
void HLCD_Cursor(bool enable)
{
    if (enable)
        lcd1602_ctrl |= LCD1602_CURSORON;
    else
        lcd1602_ctrl &= ~LCD1602_CURSORON;
    lcd1602_command(LCD1602_DISPLAYCONTROL | lcd1602_ctrl);
}

/* set text blink */
void HLCD_Blink(bool enable)
{
    if (enable)
        lcd1602_ctrl |= LCD1602_BLINKON;
    else
        lcd1602_ctrl &= ~LCD1602_BLINKON;
    lcd1602_command(LCD1602_DISPLAYCONTROL | lcd1602_ctrl);
}
