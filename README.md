# STM32F103C8T6
This is my journey into teaching myself how to program the STM32F1 without using ST's SPL or HAL library ("bare metal") to gain a better understanding of how it works. I use the cheap "STM32F103C8T6 dev boards" which can be easily found online for <5€.

There is a simple example for each peripheral, so far including:
* Timer interrupts
* PWM
* ADC
* DMA with ADC
* DMA with PWM to make a PWM DAC
* I2C
* USART

"INIT" contains the initialization routine called at startup which sets up the clock and the SysTick counter. It also provides a simple delay/wait fuction that uses the SysTick Timer.
