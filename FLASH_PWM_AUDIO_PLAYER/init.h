/*	initialization routines for the blue STM32F103C8T6 dev board
 *  to configure the clock at max speed and provide a delay function
 *  using the SysTick timer
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
/*	initialization routines for the blue STM32F103C8T6 dev board
 *  to configure the clock at max speed and provide a delay function
 *  using the SysTick timer
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#ifndef INIT_H_
#define INIT_H_

#include "stm32f1xx.h"

// main clock frequency
uint32_t SystemCoreClock;
// counter variable for the Systick counter
volatile uint32_t sysTick_Time;

// SysTick interrupt handler for counting the ticks
void SysTick_Handler(void);
// configure system clock to 72MHz using the 8MHz external crystal
// this function is called from within <startup_stm32.s>
void SystemInit(void);
// delay (i.e. wait) for a certain time
// the unit of the delays (e.g. ms or µs) is defined by the core function "SysTick_Config"
void delay(uint32_t delayTime);

#endif /* INIT_H_ */
