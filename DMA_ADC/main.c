/*	example of using the DMA controller to read the ADC
 *
 *	the program periodically reads the values from the ADC
 *	into a buffer. When the buffer is full, the average value of
 *	all values in the array is calculated and printed via the serial port.
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#include "stm32f1xx.h"
#include "init.h"
#include "usart1.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUF_SIZE 256

volatile uint16_t adc_buffer[BUF_SIZE];
volatile uint32_t average = 0;
volatile bool new_average_available = false;


void init_ADC1_DMA( uint8_t channel ){
	/* CLOCK & ADC SETUP */

	// set ADC clock prescaler to 6 to get a 12MHZ ADC clock (14MHz is maximum)
	RCC->CFGR &=~RCC_CFGR_ADCPRE_0;
	RCC->CFGR |= RCC_CFGR_ADCPRE_1;
	// enable ADC1 clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// set sample time to 239.5+12.5 cycles thus achieving (239.5+12.5)/12MHz=21µs sample rate
	ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0);
	// set data alignment to right
	ADC1->CR2 &=~ADC_CR2_ALIGN;
	// enable continuous conversion and DMA mode and turn on ADC1
	ADC1->CR2 |= (ADC_CR2_CONT | ADC_CR2_DMA | ADC_CR2_ADON);

	/* CALIBRATION */

	// delay 10µs before ADC calibration
	delay(10);
	// start calibration of ADC1
	ADC1->CR2 |= ADC_CR2_CAL;
	// wait until calibration is finished
	while( (ADC1->CR2) & ADC_CR2_CAL );

	/* ADC CHANNEL SELECTION */

	// check if channel parameter is within range
	if( (channel>17) || (channel<0) ) return;
	// set number of ADC channels to scan to 1 (so we read only 1 channel)
	ADC1->SQR1 = 0;
	// set the channel that we want to use
	ADC1->SQR3 = (channel << ADC_SQR3_SQ1_Pos);

	/* DMA SETUP */

	//enable DMA1 clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// set the peripheral address from where to fetch the data
	DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR));
	// set the destination memory address where to copy the data, i.e the buffer
	DMA1_Channel1->CMAR = (uint32_t) adc_buffer;
	// set the number of data to be transferred (i.e. the number of values in our buffer)
	DMA1_Channel1->CNDTR = BUF_SIZE;
	// set the DMA channel priority to "low"
	DMA1_Channel1->CCR &=~ (DMA_CCR_PL_1 | DMA_CCR_PL_0);
	// enable circular mode
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;
	// enable memory increment mode
	DMA1_Channel1->CCR |= DMA_CCR_MINC;
	// set destination memory size to 16bit
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;
	// set peripheral size to 16bit
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;
	// enable transfer complete interrupt
	DMA1_Channel1->CCR |= DMA_CCR_TCIE;
	// enable DMA1 interrupt
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	// globally enable interrupts
	__enable_irq();
	// enable the DMA channel
	DMA1_Channel1->CCR |= DMA_CCR_EN;
	//start AD conversion
	ADC1->CR2 |= ADC_CR2_ADON;
}

// this is triggered when our adc_buffer is full
void DMA1_Channel1_IRQHandler(){
	// check if the "transfer complete event" has triggered the interrupt
	if( DMA1->ISR & DMA_ISR_TCIF1 ){
		// calculate a new average value
		average = 0;
		for(uint8_t i=0; i<255; ++i){
			average += adc_buffer[i];
		}
		average = average >> 8;
		new_average_available = true;
	}
	// clear the interrupt flag
	DMA1->IFCR |= DMA_IFCR_CGIF1;

}

int main(void) {
	// Initialize system timer for 1µs ticks
	SysTick_Config(SystemCoreClock / 1000000UL);
	//init_USART1();
	// set interrupt priority of SysTick to 0 (highest)
	NVIC_SetPriority(SysTick_IRQn, 0);

	init_USART1();

	// set PA0 as input in analog mode
	GPIOA->CRL &=~(GPIO_CRL_MODE0_0 | GPIO_CRL_MODE0_1 | GPIO_CRL_CNF0_0 | GPIO_CRL_CNF0_1);
	// init ADC1 channel 0(PA0) and setup DMA
	init_ADC1_DMA(0);

	char adc_result_str[5];
	while (1) {
		if(new_average_available){
			itoa(average, adc_result_str, 10);
			USART1_transmitString(adc_result_str);
			USART1_transmit('\n');
			new_average_available = false;
		}
	}

}
