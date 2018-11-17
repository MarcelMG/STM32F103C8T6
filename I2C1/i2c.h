/*	minimalist library for using the I2C1 on the STM32F103C8T6 µC
 *  in polling mode and standard speed(100kHz) mode
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#ifndef I2C_H_
#define I2C_H_

#include "stm32f1xx.h"

// set this to 1 to remap the SDA/SCL pins from PB6/7 to PB8/9
#define REMAP 0
// set this to 1 to use fast mode with f=360kHz, else use standard mode with f=100kHz
#define FAST_MODE 1
// error codes
#define BUS_ERROR 			(1<<0)
#define ARBITRATION_LOSS 	(1<<1)
#define ACKNOWLEDGE_FAILURE (1<<2)
#define OVERRUN_UNDERRUN	(1<<3)
#define PEC_ERROR			(1<<4)
#define TIMEOUT_TLOW_ERROR	(1<<5)
// variable for the error code
volatile uint8_t i2c1_error;
// error interrupt handler
void I2C1_ER_IRQHandler();
// initialization of I2C1
void init_i2c1();
// read N bytes
uint8_t i2c_read( uint8_t slave_address, uint8_t* data, uint8_t N );
// write N bytes
uint8_t i2c_write( uint8_t slave_address, uint8_t* data, uint8_t N );


#endif /* I2C_H_ */
