/*	example on how to make a 8bit PWM DAC (Digital to Analog Converter)
 *  and play a test audio signal
 *
 * 	-> Timer 2 is used for PWM generation with 280kHz in left-aligned mode
 * 	-> Timer 1 is used to trigger the DMA at a regular frequency (the sampling frequency)
 *	-> DMA transfers the duty-cycle values (the 'amplitude' of the generated signal)
 *	   to the corresponding register of timer 2
 *
 * 	connect the output pin PA0 to a RC-low-pass filter
 * 	e.g. R=100R and C=100nF give ~16kHz cutoff frequency
 *
 *	the sampling frequency is almost exactly 44.1kHz (which is the standard value for audio)
 *	the analog filter should dampen all frequencies > 22.05kHz (half of sampling frequency)
 *	that are outside the audible spectrum
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 * */

#include "stm32f1xx.h"
#include "init.h" //contains the system clock setup and SysTick init

#include "hello.h"	//contains a test audio snippet

const uint8_t sine_LUT[9] = {128,209,253,238,171,84,17,2,46}; // samples of a sine wave

int main(void)
{
	// Initialize system timer for 1µs ticks
	// i.e. the delay() function uses units of 1µs
	SysTick_Config(SystemCoreClock / 1000000);


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


	/* DMA SETUP */

	//enable DMA1 clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// set the peripheral address where to put the data
	// in this case it's the timers compare (duty-cycle) register
	DMA1_Channel5->CPAR = (uint32_t) (&(TIM2->CCR1));
	// set the memory address from where to fetch the data
	// this is the starting address of our array of samples
	DMA1_Channel5->CMAR = (uint32_t) hello;
	//DMA1_Channel5->CMAR = (uint32_t) sine_LUT;
	// set the number of data (samples) to be transferred
	DMA1_Channel5->CNDTR = HELLO_LENGTH;
	//DMA1_Channel5->CNDTR = 9; //for sine_LUT
	// set data transfer direction to "memory->peripheral"
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	// set the DMA channel priority to "high"
	DMA1_Channel5->CCR |= DMA_CCR_PL_1;
	// enable circular mode (this means it will be repeated eternally)
	//DMA1_Channel5->CCR |= DMA_CCR_CIRC;
	// enable memory increment mode
	DMA1_Channel5->CCR |= DMA_CCR_MINC;
	// disable peripheral increment mode
	DMA1_Channel5->CCR &=~DMA_CCR_PINC;
	// set memory size to 8bit
	DMA1_Channel5->CCR &=~DMA_CCR_MSIZE;
	// set peripheral size to 16bit
	DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
	// enable the DMA channel
	DMA1_Channel5->CCR |= DMA_CCR_EN;


	/* TIMER 1 SETUP */

	// enable the clock for Timer1 (f_PCLK2=72MHz)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// use a prescaler of 1, i.e. f_TIM1=f_PCLK2/(0+1)=72MHz
	TIM1->PSC = 0;
	// set timer1 auto-reload value
	// so the timer overflow will occur with a frequency of 72MHz/1633=approx. 44.1kHz
	TIM1->ARR = 1633;
	// enable timer update DMA request
	// i.e. at every counter update/overflow, a DMA request will be issued
	// which triggers a DMA data transfer
	TIM1->DIER |= TIM_DIER_UDE;
	// start timer1
	TIM1->CR1 |= TIM_CR1_CEN;


	/* MAIN LOOP */
	while(1){
		// do nothing
	}

}
