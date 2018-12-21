/*	Timer1 example on STM32F103C8T6
 *  toggling an LED on pin PB12
 * */

#include "stm32f1xx.h"
#include "init.h" //contains the system clock setup and SysTick init

// timer1 update interrupt handler
void TIM1_UP_IRQHandler(){
	// toggle LED
	GPIOB->ODR ^= GPIO_ODR_ODR12;
	// clear timer1 update interrupt flag
	TIM1->SR &=~TIM_SR_UIF;
}

int main(void)
{
	// Initialize system timer for 1ms ticks
	SysTick_Config(SystemCoreClock / 1000);

	// pin PB12 as push-pull-output to drive an LED
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRH &=~ (GPIO_CRH_MODE12 | GPIO_CRH_CNF12);
	GPIOB->CRH |= GPIO_CRH_MODE12;

	// enable the clock for Timer1 (f_PCLK2=72MHz)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// use a prescaler of 7200, i.e. f_TIM1=f_PCLK2/7200=10kHz
	TIM1->PSC = 7199;
	// set timer1 auto-reload value
	TIM1->ARR = 5000;
	// clear the timer1 flags
	TIM1->SR = 0;
	// enable timer1 update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	// globally enable interrupts
	__enable_irq();
	// start timer1
	TIM1->CR1 |= TIM_CR1_CEN;

	while(1){
		// do nothing
	}
}
