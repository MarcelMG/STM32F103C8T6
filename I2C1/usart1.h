/*	minimalist library for using the USART1 on the STM32F103C8T6 µC
 *  in polling mode with a fixed baud rate
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#ifndef USART1_H_
#define USART1_H_

#include "stm32f1xx.h"

void init_USART1(void);
char USART1_receive(void);
void USART1_transmit(char data);
void USART1_transmitString(char* data_string);

#endif /* USART1_H_ */
