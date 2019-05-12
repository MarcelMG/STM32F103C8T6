/*	audio playback with PWM and the Winbond W25Q64JV SPI flash memory
 *
 *  connection:
 *	PB3 -> CLK
 *	PB4 -> MISO (DO/IO1)
 *	PB5 -> MOSI (DI/IO0)
 *	PA4 -> CS
 *	Vdd -> 3.3V
 *	Vref-> 3.3V
 *	PA0 -> PWM output
 *
 *	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#include "stm32f1xx.h"
#include "init.h"
#include "W25Q64JV.h"

#define address 000000

// timer1 update interrupt handler
void TIM1_UP_IRQHandler(){
	TIM2->CCR1 = SPI_transmit(0xFF);
	// clear timer1 update interrupt flag
	TIM1->SR &=~TIM_SR_UIF;
}

int main(void)
{
	// Initialize system timer for 1ms ticks (else divide by 1e6 for µs ticks)
	SysTick_Config(SystemCoreClock / 1e3);

	init_W25Q64JV();
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(FAST_READ);
	// send 24bit address MSB first
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	//send 8 dummy clocks, i.e. 1 dummy byte
	SPI_transmit(0xFF);


	/* TIMER 2 SETUP FOR PWM */

	// enable timer 2 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// enable AFIO and GPIO port A clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
	// configure pin PA0 as AFIO push-pull output with max speed
	GPIOA->CRL &=~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	GPIOA->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1;
	// set timer 2 prescaler to 1, so the timer frequency is equal to the APB2 frequency (here 72MHz)
	// the actual prescaling factor is PSC+1
	TIM2->PSC = 0;
	// set the auto-reload value, i.e. the max counter value
	TIM2->ARR = 255;
	// set the output compare value which determines the duty cycle
	TIM2->CCR1 = 128;
	// configure channel 1 to PWM mode 1 together with the preload feature
	// refer to ST's app note "AN4776" p.14-16 for details about preload
	// PWM mode 1: duty_cycle = CCRx / ARR
	// PWM mode 2: duty_cycle = 1 - CCRx / ARR
	// if the counter is not up- but downcounting it is inverted
	// by default, the timers are upcounting
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
	// set polarity of the output to non-inverted (default, so actually not necessary)
	TIM2->CCER &=~TIM_CCER_CC1P;
	// enable the compare outputs
	TIM2->CCER |= TIM_CCER_CC1E;
	// left-aligned mode is default

	/* 	enable center-aligned mode 3
 	 	in center-aligned mode the PWM frequency is only half as high as in edge aligned mode
	 	so here f_PWM = f_APB2 / ( 2*(PSC+1)*ARR ) = approx. 140.6 kHz
	*/
	//TIM2->CR1 |= TIM_CR1_CMS;

	// enable the timer 2 counter
	TIM2->CR1 |= TIM_CR1_CEN;


	/* TIMER 1 SETUP */

	// enable the clock for Timer1 (f_PCLK2=72MHz)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// use a prescaler of 1, i.e. f_TIM1=f_PCLK2/(0+1)=72MHz
	TIM1->PSC = 0;
	// set timer1 auto-reload value
	// so the timer overflow will occur with a frequency of 72MHz/1633=approx. 44.1kHz
	TIM1->ARR = 1633;
	// enable timer1 update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	// globally enable interrupts
	__enable_irq();
	// start timer1
	TIM1->CR1 |= TIM_CR1_CEN;





}
