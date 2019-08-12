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
#define I2C1_REMAP 0
// set this to 1 to use fast mode with f=360kHz, else use standard mode with f=100kHz
#define I2C1_FAST_MODE 0
// error codes to compare against return value "i2c1_error" return by i2c_read() or i2c_write()
#define I2C_BUS_ERROR 			(1<<0)
#define I2C_ARBITRATION_LOSS 	(1<<1)
#define I2C_ACKNOWLEDGE_FAILURE (1<<2)
#define I2C_OVERRUN_UNDERRUN	(1<<3)
#define I2C_PEC_ERROR			(1<<4)
#define I2C_TIMEOUT_TLOW_ERROR	(1<<5)
#define I2C_TIMEOUT_GENERAL		(1<<6)
// timeout in ms to prevent getting stuck in case of error
#define I2C_TIMEOUT 10
// error flag checking
uint8_t i2c1_check_error();
// initialization of I2C1
void init_i2c1();
// read N bytes
uint8_t i2c_read( uint8_t slave_address, uint8_t* data, uint8_t N );
// write N bytes
uint8_t i2c_write( uint8_t slave_address, uint8_t* data, uint8_t N );


#endif /* I2C_H_ */
