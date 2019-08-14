/*	minimalist library for using the I2C1 on the STM32F103C8T6 µC
 *  in polling mode and standard speed(100kHz) mode
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#include "i2c.h"
#include "init.h" // for SysTick_Time
// check the error flag and set the corresponding error code,
// then clear the corresponding flag
uint8_t i2c1_check_error(){
	uint8_t i2c1_error = 0;
	if (I2C1->SR1 & I2C_SR1_TIMEOUT){
		i2c1_error |= I2C_TIMEOUT_TLOW_ERROR;
		I2C1->SR1 &=~ I2C_SR1_TIMEOUT;
	}
	if (I2C1->SR1 & I2C_SR1_PECERR){
		i2c1_error |= I2C_PEC_ERROR;
		I2C1->SR1 &=~ I2C_SR1_PECERR;
	}
	if (I2C1->SR1 & I2C_SR1_OVR){
		i2c1_error |= I2C_OVERRUN_UNDERRUN;
		I2C1->SR1 &=~ I2C_SR1_OVR;
	}
	if (I2C1->SR1 & I2C_SR1_AF){
		i2c1_error |= I2C_ACKNOWLEDGE_FAILURE;
		I2C1->SR1 &=~ I2C_SR1_AF;
	}
	if (I2C1->SR1 & I2C_SR1_ARLO){
		i2c1_error |= I2C_ARBITRATION_LOSS;
		I2C1->SR1 &=~ I2C_SR1_ARLO;
	}
	if (I2C1->SR1 & I2C_SR1_BERR){
		i2c1_error |= I2C_BUS_ERROR;
		I2C1->SR1 &=~ I2C_SR1_BERR;
	}
	return i2c1_error;
}

/* initialize the I2C1 peripheral in fast mode with f_SCL=360kHz assuming an APB1/PCLK1 clock of 36MHz
   note: 400kHz can only be achieved when PCLK1 is a multiple of 10MHz */
void init_i2c1(){
	// enable alternate function and port B I/O peripheral clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN;
	// enable the peripheral clock for the I2C1 module
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
#if I2C1_REMAP
	// set pins PB8 & PB9 as alternate function open-drain output with max speed of 50MHz (REMAP)
	GPIOB->CRH &=~ ( GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_CNF8 | GPIO_CRH_CNF9 );
	GPIOB->CRH |= 	GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0 |GPIO_CRH_CNF8_0 | GPIO_CRH_CNF8_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0 |GPIO_CRH_CNF9_0 | GPIO_CRH_CNF9_1 ;
	AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
#else
	// set pins PB6 & PB7 as alternate function open-drain output with max speed of 50MHz
	GPIOB->CRL &=~ ( GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7 );
	GPIOB->CRL |= 	GPIO_CRL_MODE6_1 | GPIO_CRL_MODE6_0 | GPIO_CRL_CNF6_0 | GPIO_CRL_CNF6_1 | GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0 |GPIO_CRL_CNF7_0 | GPIO_CRL_CNF7_1 ;
#endif
	// reset the I2C1 peripheral
	RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
	RCC->APB1RSTR &=~RCC_APB1RSTR_I2C1RST;
	// set the APB1 clock value so the I2C peripheral can derive the correct timings
	// APB1 clock is 36MHz since SysCoreClock==72E6 and RCC_CFGR_PPRE1_DIV2==1
	I2C1->CR2 = (36 & I2C_CR2_FREQ);
#if I2C1_FAST_MODE
	// set I2C master mode to fast mode with DUTY=1 and set the CCR value
	// CCR = PCLK1[Hz] / (25*400e3[Hz]), so here CCR=36e6/(25*400e3)=3.6
	// so we have to round to the next larger value (i.e. 4) and thus can only achieve a
	// frequency of f = 360kHz (to achieve 400kHz, PCLK1 must be a multiple of 10MHz)
	I2C1->CCR = I2C_CCR_FS | I2C_CCR_DUTY | 4;
	// set the TRISE value, it is calculated as follows for fast mode: Trise = 1 + 300E-9*PCLK1[Hz]
	// so here Trise=1+300E-9*36E6=11.8, round down to 11
	I2C1->TRISE = 11;
#else
	// set I2C master mode to standard mode (100kHz) with DUTY= 0 and set the CCR value
	// CCR = PCLK1[Hz] / (2*100e3[Hz]), so here CCR=36e6/(2*100e3)=180
	I2C1->CCR = 180;
	// set the TRISE value, it is calculated as follows for standard mode: Trise = 1 + 1E-6*PCLK1[Hz]
	// so here Trise=1+36=37 (round down to nearest integer in case of fraction)
	I2C1->TRISE = 37;
#endif
	//enable the I2C1 peripheral
	I2C1->CR1 |= I2C_CR1_PE;
}


/*	read N bytes from the I2C slave
	returns 0 if no error occured
	returns an error code > 0 if an error occured	*/
uint8_t i2c_read( uint8_t slave_address, uint8_t* data, uint8_t N ){
	uint8_t i2c1_error  = 0;
	// send START condition
	I2C1->CR1 |= I2C_CR1_START;
	// wait until START has been sent
	// check for an error to prevent getting stuck in the loop
	uint32_t t1 = sysTick_Time;
	while( !(I2C1->SR1 & I2C_SR1_SB) ){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) goto stop;
	}
	// send slave address + R/W bit (1 for read)
	I2C1->DR = (slave_address << 1) | 1;
	// wait until slave address has been sent
	// check for an error to prevent getting stuck in the loop
	t1 = sysTick_Time;
	while( !(I2C1->SR1 & I2C_SR1_ADDR) ){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) goto stop;
	}
	if (N==1){
		// no acknowledge returned
		I2C1->CR1 &=~I2C_CR1_ACK;
		__disable_irq();
		// dummy readout of the SR1 and SR2 registers to clear the ADDR flag
		I2C1->SR1;
		I2C1->SR2;
		// generate STOP after the current byte transfer
		I2C1->CR1 |= I2C_CR1_STOP;
		__enable_irq();
		// wait until data receive register not empty
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( !(I2C1->SR1 & I2C_SR1_RXNE) ){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		// read the data byte
		*data = I2C1->DR;
		// wait until the STOP condition has been sent
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( I2C1->CR1 & I2C_CR1_STOP){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		// acknowledge returned (to be ready for another reception)
		I2C1->CR1 |= I2C_CR1_ACK;
	}else if(N==2){
		// ACK bit controls the (N)ACK of the next byte which will be received in the shift register
		I2C1->CR1 |= I2C_CR1_POS;
		__disable_irq();
		// dummy readout of the SR1 and SR2 registers to clear the ADDR flag
		I2C1->SR1;
		I2C1->SR2;
		// no acknowledge returned
		I2C1->CR1 &=~I2C_CR1_ACK;
		__enable_irq();
		// wait until a new byte is received (including ACK pulse) and DR has not been read yet
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( !(I2C1->SR1 & I2C_SR1_BTF) ){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		__disable_irq();
		// generate STOP after the current byte transfer
		I2C1->CR1 |= I2C_CR1_STOP;
		// read the first byte
		*data = I2C1->DR;
		__enable_irq();
		// and the second byte
		++data;
		*data = I2C1->DR;
		// wait until the STOP condition has been sent
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( I2C1->CR1 & I2C_CR1_STOP){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		// ACK bit controls the (N)ACK of the current byte being received in the shift register (default)
		I2C1->CR1 &=~I2C_CR1_POS;
		// acknowledge returned (to be ready for another reception)
		I2C1->CR1 |= I2C_CR1_ACK;
	}else if(N>2){
		// dummy readout of the SR1 and SR2 registers to clear the ADDR flag
		I2C1->SR1;
		I2C1->SR2;
		while(N>=3){
			// wait until a new byte is received (including ACK pulse) and DR has not been read yet
			// check for an error to prevent getting stuck in the loop
			t1 = sysTick_Time;
			while( !(I2C1->SR1 & I2C_SR1_BTF) ){
				i2c1_error = i2c1_check_error();
				if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
				if( i2c1_error > 0) goto stop;
			}
			// read one byte
			*data = I2C1->DR;
			++data;
			// decrement number of bytes to read
			--N;
		}
		// wait until a new byte is received (including ACK pulse) and DR has not been read yet
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( !(I2C1->SR1 & I2C_SR1_BTF) ){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		// no acknowledge returned
		I2C1->CR1 &=~I2C_CR1_ACK;
		__disable_irq();
		// generate STOP after the current byte transfer
		I2C1->CR1 |= I2C_CR1_STOP;
		// read the penultimate byte
		*data = I2C1->DR;
		++data;
		__enable_irq();
		// wait until data receive register not empty
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( !(I2C1->SR1 & I2C_SR1_RXNE) ){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		// read the last byte
		*data = I2C1->DR;
		// wait until the STOP condition has been sent
		t1 = sysTick_Time;
		while( I2C1->CR1 & I2C_CR1_STOP){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) break;
		}
		// acknowledge returned (to be ready for another reception)
		I2C1->CR1 |= I2C_CR1_ACK;
	}
	return 0; // no error
stop:
	// send STOP condition
	I2C1->CR1 |= I2C_CR1_STOP;
	// wait until STOP condition has been generated
	t1 = sysTick_Time;
	while( I2C1->CR1 & I2C_CR1_STOP){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) break;
	}
	return i2c1_error;
}

/*	write N bytes to the I2C slave
	returns 0 if no error occured
	returns an error code > 0 if an error occured	*/
uint8_t i2c_write( uint8_t slave_address, uint8_t* data, uint8_t N ){
	uint8_t i2c1_error  = 0;
	// send START condition
	I2C1->CR1 |= I2C_CR1_START;
	// wait until START has been sent
	// check for an error to prevent getting stuck in the loop
	uint32_t t1 = sysTick_Time;
	while( !(I2C1->SR1 & I2C_SR1_SB) ){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) goto stop;
	}
	// send slave address + R/W bit (0 for write)
	I2C1->DR = (slave_address << 1) | 0;
	// wait until slave address has been sent
	// check for an error to prevent getting stuck in the loop
	t1 = sysTick_Time;
	while( !(I2C1->SR1 & I2C_SR1_ADDR) ){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) goto stop;
	}
	// dummy readout of the SR1 and SR2 registers to clear the ADDR flag
	I2C1->SR1;
	I2C1->SR2;
	// send N bytes
	while( N>0 ){
		// write the byte to be sent into the data register
		I2C1->DR = *data;
		// wait until data byte transfer succeeded
		// check for an error to prevent getting stuck in the loop
		t1 = sysTick_Time;
		while( !(I2C1->SR1 & I2C_SR1_BTF) ){
			i2c1_error = i2c1_check_error();
			if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
			if( i2c1_error > 0) goto stop;
		}
		++data;
		--N;
	}
stop:
	// send STOP condition
	I2C1->CR1 |= I2C_CR1_STOP;
	// wait until STOP condition has been generated
	t1 = sysTick_Time;
	while( I2C1->CR1 & I2C_CR1_STOP){
		i2c1_error = i2c1_check_error();
		if( (sysTick_Time - t1) > 10 ) i2c1_error |= I2C_TIMEOUT_GENERAL;
		if( i2c1_error > 0) break;
	}
	return i2c1_error;
}
