 /*	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#include "W25Q64JV.h"

// this function is the only one that has to be modified when porting to another platform
// it has to set up the SPI and the GPIO needed for SPI(including one for the CS line)
void init_W25Q64JV(){
	// f_SPI = 72MHz/SPI_BAUD_DIV_X
	init_SPI1(false, (SPI_MODE_0 | SPI_MSB_FIRST | SPI_8BIT_FRAME | SPI_BAUD_DIV_32) );
	// setup a GPIO pin, e.g. PA4 as output for the CS(chip select) line
	// enable clock for GPIO port
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// set GPIO pin as push-pull output with max speed 50MHz
	GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
	GPIOA->CRL |= GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0;
	// CS high
	CS_HIGH();
}

void power_down_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(POWER_DOWN);
	// CS high, transmission finished
	CS_HIGH();
	// it takes >3µs for the W25Q64JV to wake up from power-down mode
	// delay(3µs)
}

void power_up_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(RELEASE_PWR_DWN_ID);
	// CS high, transmission finished
	CS_HIGH();
	// it takes >3µs for the W25Q64JV to wake up from power-down mode
	// delay(3µs)
}

void read_W25Q64JV(uint32_t address, uint32_t length, uint8_t* destination_ptr){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(READ_DATA);
	// send 24bit address MSB first
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	for(uint32_t byte_counter = 0; byte_counter<length; byte_counter++){
		destination_ptr[byte_counter] = SPI_transmit(0xFF);
	}
	// CS high, transmission finished
	CS_HIGH();
}

void fast_read_W25Q64JV(uint32_t address, uint32_t length, char* destination_ptr){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(FAST_READ);
	// send 24bit address MSB first
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	//send 8 dummy clocks, i.e. 1 dummy byte
	SPI_transmit(0xFF);
	for(uint32_t byte_counter = 0; byte_counter<length; byte_counter++){
		destination_ptr[byte_counter] = SPI_transmit(0xFF);
	}
	// CS high, transmission finished
	CS_HIGH();
}

// write a page of 1-256bytes to previously erased(!!!) locations
void write_W25Q64JV(uint32_t address, uint16_t length, uint8_t* source_ptr){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(WRITE_ENABLE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(PAGE_PROGRAM);
	// send 24bit address MSB first
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	// from datasheet: If an entire 256 byte page is to be programmed, the last address byte (the 8 LSB) should be set to 0.
	if(length == 256){
		SPI_transmit(0);
	}else{
		SPI_transmit( (uint8_t)(address) );
	}
	for(uint32_t byte_counter = 0; byte_counter<length; byte_counter++){
		SPI_transmit(source_ptr[byte_counter]);
	}
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// poll status register 1 to check the BUSY bit which indicates that writing procedure is over
	wait_busy_flag_W25Q64JV();
	CS_HIGH();
}


// erases a sector of 4Kbytes starting from the address specified
// then wait until the BUSY bit in status register 1 is cleared, i.e. erasing is finished
void sector_erase_W25Q64JV(uint32_t address){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(WRITE_ENABLE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(SECTOR_ERASE_4KB);
	// send 24bit address of block to be erased (MSB first)
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// poll status register 1 to check the BUSY bit which indicates that erase procedure is over
	wait_busy_flag_W25Q64JV();
	// CS high, transmission finished
	CS_HIGH();
}

// erases a block of 32Kbytes starting from the address specified
// then wait until the BUSY bit in status register 1 is cleared, i.e. erasing is finished
void block_erase_32KB_W25Q64JV(uint32_t address){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(WRITE_ENABLE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(BLOCK_ERASE_32KB);
	// send 24bit address of block to be erased (MSB first)
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// poll status register 1 to check the BUSY bit which indicates that erase procedure is over
	wait_busy_flag_W25Q64JV();
	// CS high, transmission finished
	CS_HIGH();
}

// erases a block of 64Kbytes starting from the address specified
// then wait until the BUSY bit in status register 1 is cleared, i.e. erasing is finished
void block_erase_64KB_W25Q64JV(uint32_t address){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(WRITE_ENABLE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(BLOCK_ERASE_64KB);
	// send 24bit address of block to be erased (MSB first)
	SPI_transmit( (uint8_t)(address>>16) );
	SPI_transmit( (uint8_t)(address>>8) );
	SPI_transmit( (uint8_t)(address) );
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// poll status register 1 to check the BUSY bit which indicates that erase procedure is over
	wait_busy_flag_W25Q64JV();
	// CS high, transmission finished
	CS_HIGH();
}

void erase_chip_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(WRITE_ENABLE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(CHIP_ERASE);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// poll status register 1 to check the BUSY bit which indicates that erase procedure is over
	wait_busy_flag_W25Q64JV();
	// CS high, transmission finished
	CS_HIGH();
}

// read the chips unique ID (64bit)
uint64_t get_unique_ID_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(READ_UNIQUE_ID);
	// 4 dummy clock bytes
	SPI_transmit(0xFF);
	SPI_transmit(0xFF);
	SPI_transmit(0xFF);
	SPI_transmit(0xFF);
	// receive ID byte per byte and assemble the 64bit value from the bytes
	uint8_t byte_buf;
	uint64_t ID = 0;
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 56 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 48 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 40 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 32 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 24 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 16 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 8 );
	byte_buf = SPI_transmit(0xFF);
	ID |= ( ((uint64_t)byte_buf) << 0 );
	// CS high, transmission finished
	CS_HIGH();
	return ID;
}

void reset_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(ENABLE_RESET);
	// CS high, transmission finished
	CS_HIGH();
	//eventually a small delay is needed here, depends on your µC's speed
	asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(RESET_DEVICE);
	// CS high, transmission finished
	CS_HIGH();
}

uint8_t get_status_register1(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(READ_STATUS_REG_1);
	// receive register content
	uint8_t register_content = SPI_transmit(0xFF);
	// CS high, transmission finished
	CS_HIGH();
	return register_content;
}

void wait_busy_flag_W25Q64JV(){
	// CS low, SPI slave starts to listen
	CS_LOW();
	// send instruction
	SPI_transmit(READ_STATUS_REG_1);
	// poll status register 1 and check busy bit
	while( SPI_transmit(0xFF) & STATUS_REG_1_BUSY_BIT );
	return;
}









