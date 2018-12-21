/*	example of how to use the ADC in polling mode
 *	to perform a single conversion
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#include "stm32f1xx.h"
#include "init.h"
#include "usart1.h"
#include <stdlib.h>

void init_ADC1(){
	// set ADC clock prescaler to 6 to get a 12MHZ ADC clock (14MHz is maximum)
	RCC->CFGR &=~RCC_CFGR_ADCPRE_0;
	RCC->CFGR |= RCC_CFGR_ADCPRE_1;
	// enable ADC1 clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// enable ADC1
	ADC1->CR2 |= ADC_CR2_ADON;
	// select software start as external event trigger
	ADC1->CR2 |= (ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0);
	// set sample time to minimum (1.5µs)
	ADC1->SMPR2 &=~(ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0);
	// set data alignment to right
	ADC1->CR2 &=~ADC_CR2_ALIGN;
	// wait 10µs before ADC calibration
	delay(10);
	// start calibration of ADC1
	ADC1->CR2 |= ADC_CR2_CAL;
	// wait until calibration is finished
	while( (ADC1->CR2) & ADC_CR2_CAL );
}

uint16_t readADC( uint8_t channel ){
	// check if parameter is within range
	if( (channel>17) || (channel<0) ) return 0xFFFF;
	// set number of ADC channels to scan to 1 (so we read only 1 channel)
	ADC1->SQR1 = 0;
	// set the channel that we want to use
	ADC1->SQR3 = (channel << ADC_SQR3_SQ1_Pos);
	// clear the end of conversion flag
	ADC1->SR &=~ADC_SR_EOC;
	// start conversion
	ADC1->CR2 |= ADC_CR2_ADON;
	// trigger conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
	// wait until conversion is finished
	while( !(ADC1->SR & ADC_SR_EOC) );
	// read conversion result (12bit) from the data register and return it
	return ((ADC1->DR) & 0b111111111111);
}

int main(void) {
	// Initialize system timer for 1µs ticks
	SysTick_Config(SystemCoreClock / 1000000UL);
	init_USART1();

	// set PA0 as input in analog mode
	GPIOA->CRL &=~(GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1 | GPIO_CRL_CNF0_0 | GPIO_CRL_CNF0_1);
	init_ADC1();
	char adc_result_str[5];
	while (1) {
		// perform ADC and convert result into C-string
		itoa(readADC(0), adc_result_str, 10);
		// transmit result via serial port
		USART1_transmitString(adc_result_str);
		USART1_transmit('\n');
		delayms(100);
	}
}
