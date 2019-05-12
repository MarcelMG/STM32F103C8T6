/*  simple routines for using SPI in polling mode
 *  for the STM32F103
 *
 *	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#include "SPI.h"

uint8_t SPI1_error = 0;
uint8_t SPI2_error = 0;

/* enable and initialize the SPI1 peripheral
arguments:		remap 	= 	remap the pins (true or false)
				config	=	(SPI_BAUD_DIV_x | SPI_MODE_x | SPI_xSB_FIRST | SPI_8BIT_FRAME)*/
void init_SPI1(bool remap, uint16_t config){
	// enable the clock for the SPI peripheral
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	// configure the AFIO pins for MOSI, MISO and SCK
	if(remap){
		// enable clock for GPIO port
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
		// remap SPI1
		AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;
		// configure the pins:
		GPIOB->CRL &=~ 	(GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_CNF3 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5 );
		// configure PB3(SCK1) & PB5(MOSI1) as AFIO push-pull outputs
		// and PB4(MISO1) as input with pullup
		GPIOB->CRL |= GPIO_CRL_MODE3 | GPIO_CRL_MODE5 | GPIO_CRL_CNF3_1 | GPIO_CRL_CNF5_1 | GPIO_CRL_CNF4_1;
		// activate pullup on PB4
		GPIOB->ODR |= GPIO_ODR_ODR4;
	}else{
		// enable clock for GPIO port
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		// configure the pins:
		GPIOA->CRL &=~ 	(GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
		// configure PA5(SCK1) & PA7(MOSI1) as AFIO push-pull outputs
		// and PA6(MISO1) as floating input
		GPIOA->CRL |= GPIO_CRL_MODE5 | GPIO_CRL_MODE7 | GPIO_CRL_CNF5_1 | GPIO_CRL_CNF6_1 |GPIO_CRL_CNF7_1 ;
	}
	// configure the SPI mode (i.e. clock polarity & phase), data frame format and clock speed (<config>)
	// set in in Master mode and set NSS to software mode (i.e. the slave select is optional and controlled externally by a GPIO pin)
	// then enable the SPI peripheral
	SPI1->CR1 |= config | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;
}

/* enable and initialize the SPI1 peripheral
argument:	config	=	(SPI_BAUD_DIV_x | SPI_MODE_x | SPI_xSB_FIRST | SPI_8BIT_FRAME)*/
void init_SPI2(uint16_t config){
	// enable the clock for the SPI peripheral
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	// enable clock for GPIO port(s)
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	GPIOB->CRH &=~ (GPIO_CRH_MODE12 | GPIO_CRH_CNF12 | GPIO_CRH_MODE13 | GPIO_CRH_CNF13 | GPIO_CRH_MODE14 | GPIO_CRH_CNF14 | GPIO_CRH_MODE15 | GPIO_CRH_CNF15);
	// configure PB13(SCK2) and PB15(MOSI2) as AFIO push-pull
	// outputs and PB14(MISO2) as input with pullup
	GPIOB->CRH |= GPIO_CRH_MODE13 | GPIO_CRH_CNF13_1 | GPIO_CRH_CNF14_1 | GPIO_CRH_MODE15 | GPIO_CRH_CNF15_1;
	// activate pullup on PB14
	GPIOB->ODR |= GPIO_ODR_ODR14;
	// configure the SPI mode (i.e. clock polarity & phase), data frame format and clock speed (<config>)
	// set in in Master mode and set NSS to software mode (i.e. the slave select is optional and controlled externally by a GPIO pin)
	// then enable the SPI peripheral
	SPI2->CR1 = config | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_SPE;
}

// change the SPI clock speed by changing the clock divider
void SPI1_set_clock_div(uint8_t divider){
	if(divider > 7){return;}
	SPI1->CR1 &=~ SPI_CR1_BR;
	SPI1->CR1 |= divider;
}

// change the SPI clock speed by changing the clock divider
void SPI2_set_clock_div(uint8_t divider){
	if(divider > 7){return;}
	SPI2->CR1 &=~ SPI_CR1_BR;
	SPI2->CR1 |= divider;
}

// send one/two byte(s) to the slave and receive one/two byte(s)
uint16_t SPI1_transmit(uint16_t tx_data){
	// put the data byte(s) to be sent into the buffer
	SPI1->DR = tx_data;
	// wait until the byte(s) has been transmitted
	uint32_t t1 = sysTick_Time;
	while( !(SPI1->SR & SPI_SR_TXE) ){
		// check time to prevent getting stuck in the loop in case of an error
		// exit loop after 0.1s if nothing happens
		if( sysTick_Time > (t1+100) ){
			SPI1_error = 1;
			return 0xFFFF;
		}
	}
	// wait until the response has been received
	while( !(SPI1->SR & SPI_SR_RXNE) ){
		// check time to prevent getting stuck in the loop in case of an error
		// exit loop after 0.1s if nothing happens
		if( sysTick_Time > (t1+100) ){
			SPI1_error = 1;
			return 0xFFFF;
		}
	}
	// return the data byte received from the slave
	return (uint16_t) SPI1->DR;
}

// send one/two byte(s) to the slave and receive one/two byte(s)
uint16_t SPI2_transmit(uint16_t tx_data){
	// put the data byte(s) to be sent into the buffer
	SPI2->DR = tx_data;
	// wait until the byte(s) has been transmitted
	uint32_t t1 = sysTick_Time;
	while( !(SPI2->SR & SPI_SR_TXE) ){
		// check time to prevent getting stuck in the loop in case of an error
		// exit loop after 0.1s if nothing happens
		if( sysTick_Time > (t1+100) ){
			SPI2_error = 1;
			return 0xFFFF;
		}
	}
	// wait until the response has been received
	while( !(SPI2->SR & SPI_SR_RXNE) ){
		// check time to prevent getting stuck in the loop in case of an error
		// exit loop after 0.1s if nothing happens
		if( sysTick_Time > (t1+100) ){
			SPI2_error = 1;
			return 0xFFFF;
		}
	}
	// return the data byte received from the slave
	return (uint16_t) SPI2->DR;
}










