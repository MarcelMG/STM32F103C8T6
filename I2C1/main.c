/* example of using the i2c library with an QMC5883L magnetometer module
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */


#include "stm32f1xx.h"
#include "init.h"
#include "i2c.h"
#include <stdlib.h>

#define QMC5883L_ADRESS 0x0D
const uint8_t X_DATA_LSB = 0x00;
const uint8_t X_DATA_MSB = 0x01;
const uint8_t Y_DATA_LSB = 0x02;
const uint8_t Y_DATA_MSB = 0x03;
const uint8_t Z_DATA_LSB = 0x04;
const uint8_t Z_DATA_MSB = 0x05;
const uint8_t TEMP_DATA_LSB = 0x07;
const uint8_t TEMP_DATA_MSB = 0x08;
const uint8_t CONTROL_REGISTER_1 = 0x09;
const uint8_t STATUS_REGISTER_1 = 0x06;
const uint8_t SET_RESET_PERIOD_REGISTER = 0x0B;
uint8_t buf[2];
int16_t x_data = 0;
int16_t y_data = 0;
int16_t z_data = 0;
char strbuf[10];
uint8_t i2c_err;

int main(void){
	// Initialize system timer for 1ms ticks (else divide by 1e6 for µs ticks)
	SysTick_Config(SystemCoreClock / 1000);
	// init the USART1 peripheral to print to serial terminal
	init_USART1();
	// init the I2C1 peripheral and the SDA/SCL GPIO pins
	init_i2c1();
	USART1_transmitString("testing QMC5883L:\n");
	// init QMC5883L
	buf[0] = SET_RESET_PERIOD_REGISTER;
	buf[1] = 0x01;
	i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 2);
	check_i2cerr(i2c_err);
	buf[0] = CONTROL_REGISTER_1;
	// continuous mode, 200Hz data rate, 2G scale, 64 oversampling ratio
	buf[1] = 0b11001101;
	i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 2);
	check_i2cerr(i2c_err);
	while(1){
		// read x-data
		buf[0] = X_DATA_LSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		x_data = buf[0];
		buf[0] = X_DATA_MSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		x_data |= (buf[0] << 8);
		// read y-data
		buf[0] = Y_DATA_LSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		y_data = buf[0];
		buf[0] = Y_DATA_MSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		y_data |= (buf[0] << 8);
		// read z-data (NOTE: if we don't read the z-data MSB, the QMC5883L won't
		//update the values, so we have to read it even if we don't need it
		buf[0] = Z_DATA_LSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		z_data = buf[0];
		buf[0] = Z_DATA_MSB;
		i2c_err = i2c1_write(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		i2c_err = i2c1_read(QMC5883L_ADRESS, buf, 1);
		check_i2cerr(i2c_err);
		z_data |= (buf[0] << 8);
		// print the result over USART1
		itoa(x_data, strbuf, 10);
		USART1_transmitString("x = ");
		USART1_transmitString(strbuf);
		itoa(y_data, strbuf, 10);
		USART1_transmitString(" y = ");
		USART1_transmitString(strbuf);
		itoa(z_data, strbuf, 10);
		USART1_transmitString(" z = ");
		USART1_transmitString(strbuf);
		USART1_transmit('\n');
		delay(300);
	}
return 1;
}
