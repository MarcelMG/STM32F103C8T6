#include "init.h"


// at startup the default configuration uses the internal 8MHz clock
// so the main clock frequency is 8MHz
uint32_t SystemCoreClock = 8000000;
// counter variable for the Systick counter in ms
volatile uint32_t sysTick_Time = 0;

// SysTick interrupt handler for counting the ticks
void SysTick_Handler(void) {
	sysTick_Time++;
}

// configure system clock to 72MHz using the 8MHz external crystal
// this function is called from within <startup_stm32.s>
void SystemInit(void) {
	// set flash latency to "2 wait states", this is required for 48 MHz < SYSCLK ≤ 72 MHz
	FLASH->ACR &=~FLASH_ACR_LATENCY_0;
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	// turn the HSE oscillator (external 8MHz crystal) on
	RCC->CR |= RCC_CR_HSEON;
	// wait until the HSE oscillator (external 8MHz crystal) is stable
	while (!(RCC->CR & RCC_CR_HSERDY));
	// select 8MHz HSE oscillator as PLL input, set PLL multiplication factor to 9
	// and divide APB1 clock by 2 so that it doesn't exceed 36MHz
	RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PLLMULL9;
	// enable the PLL (this has to be done after setting the PLL multiplication factor)
	RCC->CR |= RCC_CR_PLLON;
	// wait until the PLL is locked (i.e. ready)
	while (!(RCC->CR & RCC_CR_PLLRDY));
	// Select PLL as system clock source
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	// update the system core clock to its new value 72MHz
	SystemCoreClock = 72000000;
}


// delay (i.e. wait) for a certain time
// the unit of the delays (e.g. ms or µs) is defined by the core function "SysTick_Config"
void delay(uint32_t delayTime){
	uint32_t startTime =  sysTick_Time;
	while ( (sysTick_Time - startTime) < delayTime );
}
