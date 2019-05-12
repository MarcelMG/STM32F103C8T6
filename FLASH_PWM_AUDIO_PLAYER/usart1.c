/*
 * usart1.c
 *
 *  Created on: 11.09.2018
 *      Author: marcel
 */
#include "usart1.h"

// initialize the USART with default settings (1start, 8data, 1stop bit) and 9600 baud rate
void init_USART1(void) {
	//	enable GPIO port A clock , alternate function I/O clock and USART1 clock
	RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN  | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN);
	// enable USART, transmitter and receiver
	USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
	// 	set baud rate divider to achieve a baud rate of 115200 from the 72MHz PCLK2
	USART1->BRR = 650;	// alternatively e.g. BRR = 7500 for 9600 baud
	// 	set PA9 as alternate function push-pull output with max speed of 50MHz
	GPIOA->CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1);
	// 	set PA10 as floating input
	GPIOA->CRH &= ~(GPIO_CRH_CNF10_1 | GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1);
	GPIOA->CRH |= GPIO_CRH_CNF10_0;
}

char USART1_receive(void) {
	// wait until the USART data register is not empty
	while (!((USART1->SR) & USART_SR_RXNE));
	// return the content (first byte) of the data register
	return ((USART1->DR) & 0xFF);
}

// WARNING: the user must make sure, that the size of "destination_str_ptr" is large enough to hold all received characters
//          or else the exceeding characters will be lost
void USART1_receiveString(char* destination_str_ptr, uint32_t length){
	for(uint32_t str_it = 0; str_it<(length-1); str_it++ ){
		uint8_t data_byte = USART1_receive();
		if ( (data_byte == '\n') || (data_byte == '\r') ) break;
		*destination_str_ptr++ = data_byte;
	}
	*destination_str_ptr = '\0';
}

void USART1_flush(){
	while ((USART1->SR) & USART_SR_RXNE){
		USART1->DR;
	}
}

void USART1_transmit(char data) {
	// 	wait until the USART data register is empty i.e. ready to transmit
	while (!((USART1->SR) & USART_SR_TXE))
		;
	//	write character to be sent into the data register
	USART1->DR = (data & 0xFF);
	// OPTIONAL: wait until transmission is completed
	while (!((USART1->SR) & USART_SR_TC));
}

void USART1_transmitString(char* data_string){
	while( *data_string != 0 ){
		USART1_transmit(*data_string++);
	}
}
