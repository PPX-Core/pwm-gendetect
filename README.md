# PWM Signal Detector and Generator

This is PWM signal generator/detector, which can detect 1 MHz ~ 2 Hz PWM signal and generate 12 MHz ~ 0.012 Hz PWM signal with variable duty ratio.

# Hardward Requirement

* STM32F030F4P6 mini development board
* LCD 1602A diaplay
* 6 resistors
* 1 potentiometer
* 1 capacitance
* 5 switches
* 1 PCB protoboard
* 2 male pin header

# PWM Detect Range
* Maximum 1 MHz
* Minimum 2 ~ 1 Hz
* Accuracy: Not measured

# PWM Output Range
* Maximum 12 MHz
* Minimum 0.012 Hz
* Duty: 0.1 ~ 99.9 % (theorically)
* Accuracy: Not measured

# Key Functions
*SW1: enable PWM output edit / digit up
*SW2: enable or disable PWM output / digit down
*SW3: enable or disable PWM input / cursor left
*SW4: disable PWM output edit / cursor right
