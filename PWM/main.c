/*	PWM example with four LEDs connected to PA0..3
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 * */

#include "stm32f1xx.h"
#include "init.h" //contains the system clock setup and SysTick init

int main(void)
{
	// Initialize system timer for 1µs ticks
	// i.e. the delay() function uses units of 1µs
	SysTick_Config(SystemCoreClock / 1000000);

	/* configure timer 2 for PWM */

	// enable timer 2 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	// enable AFIO and GPIO port A clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
	// configure pins PA0, PA1, PA2 and PA3 as AFIO push-pull outputs with max speed
	GPIOA->CRL &=~(GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 | GPIO_CRL_CNF3);
	GPIOA->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1 | GPIO_CRL_MODE1 | GPIO_CRL_CNF1_1 | GPIO_CRL_MODE2 | GPIO_CRL_CNF2_1 | GPIO_CRL_MODE3 | GPIO_CRL_CNF3_1;
	// set timer 2 prescaler to 1, so the timer frequency is equal to the APB2 frequency (here 36MHz)
	// the actual prescaling factor is PSC+1
	TIM2->PSC = 0;
	// set the auto-reload value, i.e. the max counter value
	// this determines our PWM frequency together with the PSC value
	// f_PWM = f_APB2 / ( (PSC+1)*ARR )
	// in this example the PWM frequency is roughly 550Hz
	TIM2->ARR = 0xFFFF;
	// set the output compare values which determine the duty cycle
	TIM2->CCR1 = 0;
	TIM2->CCR2 = 0;
	TIM2->CCR3 = 0;
	TIM2->CCR4 = 0;
	// configure channels 1 and 2 to PWM mode 1 together with the preload feature
	// refer to ST's app note "AN4776" p.14-16 for details about preload
	// PWM mode 1: duty_cycle = CCRx / ARR
	// PWM mode 2: duty_cycle = 1 - CCRx / ARR
	// if the counter is not up- but downcounting it is inverted
	// by default, the timers are upcounting
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
	// the same config for channels 3 and 4
	TIM2->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
	// set polarity of the outputs to non-inverted (default, so actually not necessary)
	TIM2->CCER &=~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
	// enable the compare outputs
	TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	// enable the timer 2 counter
	TIM2->CR1 |= TIM_CR1_CEN;


	/* demonstrate PWM with a fancy light show */
	while(1){
		for( uint16_t x = 0; x < 65535; ++x){
			// change duty cycle
			TIM2->CCR1 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 65535; x >1 ; --x){
			// change duty cycle
			TIM2->CCR1 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 0; x < 65535; ++x){
			// change duty cycle
			TIM2->CCR2 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 65535; x >1 ; --x){
			// change duty cycle
			TIM2->CCR2 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 0; x < 65535; ++x){
			// change duty cycle
			TIM2->CCR3 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 65535; x >1 ; --x){
			// change duty cycle
			TIM2->CCR3 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 0; x < 65535; ++x){
			// change duty cycle
			TIM2->CCR4 = x;
			// delay 15µs
			delay(15);
		}
		for( uint16_t x = 65535; x >1 ; --x){
			// change duty cycle
			TIM2->CCR4 = x;
			// delay 15µs
			delay(15);
		}
	}
}
