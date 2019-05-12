/*  simple routines for using SPI in polling mode
 *  for the STM32F103
 *
 *	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#ifndef SPI_H_
#define SPI_H_

#include "stm32f1xx.h"
#include <stdbool.h>
#include "init.h"

#define SPI_BAUD_DIV_2 		(0<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_4 		(1<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_8 		(2<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_16 	(3<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_32 	(4<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_64 	(5<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_128 	(6<<SPI_CR1_BR_Pos)
#define SPI_BAUD_DIV_256 	(7<<SPI_CR1_BR_Pos)
#define SPI_MODE_0			0
#define SPI_MODE_1			SPI_CR1_CPHA
#define SPI_MODE_2			SPI_CR1_CPOL
#define SPI_MODE_3			(SPI_CR1_CPOL | SPI_CR1_CPHA)
#define SPI_LSB_FIRST		SPI_CR1_LSBFIRST
#define SPI_MSB_FIRST		0
#define SPI_8BIT_FRAME		0
#define SPI_16BIT_FRAME		SPI_CR1_DFF

#define TIMEOUT 1
uint8_t SPI1_error, SPI2_error;

void init_SPI1(bool remap, uint16_t config);
void init_SPI2(uint16_t config);
void SPI1_set_clock_div(uint8_t divider);
void SPI2_set_clock_div(uint8_t divider);
uint16_t SPI1_transmit(uint16_t tx_data);
uint16_t SPI2_transmit(uint16_t tx_data);

#endif /* SPI_H_ */
