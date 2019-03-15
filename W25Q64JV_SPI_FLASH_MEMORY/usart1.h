/*
 * usart1.h
 *	simple functions for using USART1 in polling mode for serial communication
 *  Created on: 11.09.2018
 *      Author: marcel
 */

#ifndef USART1_H_
#define USART1_H_

#include "stm32f1xx.h"


void init_USART1(void);
char USART1_receive(void);
void USART1_transmit(char data);
void USART1_transmitString(char* data_string);

#endif /* USART1_H_ */
