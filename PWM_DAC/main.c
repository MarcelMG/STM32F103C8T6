/*	example on how to make a PWM DAC (Digital to Analog Converter)
 *
 * 	-> Timer 2 is used for PWM generation
 * 	-> Timer 1 is used to trigger the DMA at a regular frequency (the sampling frequency)
 *	-> DMA transfers the duty-cycle values (the 'amplitude' of the generated signal)
 *	   to the corresponding register of timer 2
 *
 * 	connect the output pin PA0 to a RC-lowpass filter
 * 	e.g. R=100R and C=100nF give ~16kHz cutoff frequency
 *
 *	here the PWM frequency is about 140kHz, and the sampling frequency
 *	almost exactly 44.1kHz (which is the standard value for audio)
 *	the analog filter should dampen all frequencies > 22.05kHz (half of sampling frequency)
 *
 *	in this example a sawtooth signal with ~4.4kHz is generated
 *	(sampling_freq / num_samples = 44.1kHz / 10 = 4.41kHz)
 *
 *	see the picture in this folder for an oscilloscope capture of the waveform
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 * */

#include "stm32f1xx.h"
#include "init.h" //contains the system clock setup and SysTick init

// triangle wave signal
volatile uint8_t samples[10] = {0, 50, 100, 150, 200, 250, 200, 150, 100, 50};
#define SAMPLE_SIZE 10

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
	// configure pins PA0, PA1, PA2 and PA3 as AFIO push-pull outputs with max speed
	GPIOA->CRL &=~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	GPIOA->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1;
	// set timer 2 prescaler to 1, so the timer frequency is equal to the APB2 frequency (here 72MHz)
	// the actual prescaling factor is PSC+1
	TIM2->PSC = 0;
	// set the auto-reload value, i.e. the max counter value
	TIM2->ARR = 255;
	// set the output compare value which determine the duty cycle
	TIM2->CCR1 = 0;
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
	// enable center-aligned mode 3
	// in center-aligned mode the PWM frequency is only half as high as in edge aligned mode
	// so here f_PWM = f_APB2 / ( 2*(PSC+1)*ARR ) = approx. 140.6 kHz
	TIM2->CR1 |= TIM_CR1_CMS;
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
	DMA1_Channel5->CMAR = (uint32_t) samples;
	// set the number of data to be transferred
	DMA1_Channel5->CNDTR = SAMPLE_SIZE;
	// set data transfer direction to "memory->peripheral"
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	// set the DMA channel priority to "high"
	DMA1_Channel5->CCR |= DMA_CCR_PL_1;
	// enable circular mode
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;
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
