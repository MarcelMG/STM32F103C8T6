/*	testing the Winbond W25Q64JV SPI flash memory
 *
 *	NOTE: when writing to the chip, the SPI clock rate
 *	should be much faster than the USART baud rate to avoid
 *	losing data (there is no buffering implemented)
 *
 *  connection:
 *	PB3 -> CLK
 *	PB4 -> MISO (DO/IO1)
 *	PB5 -> MOSI (DI/IO0)
 *	PA4 -> CS
 *	Vdd -> 3.3V
 *	Vref-> 3.3V
 *
 *	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#include "stm32f1xx.h"
#include "init.h"
#include "W25Q64JV.h"
#include "usart1.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(void)
{
	// Initialize system timer for 1ms ticks (else divide by 1e6 for µs ticks)
	SysTick_Config(SystemCoreClock / 1e3);
	init_USART1();
	init_W25Q64JV();

	while(1){
		USART1_transmitString("|--------------------------------------------|\n| W25Q64JV Flash Memory Tool |\n|--------------------------------------------|");
		USART1_transmitString("\n\nenter number to choose among the following:\n\n1 erase block (64kB)\n2 write one page (max. 256 byte)\n3 get unique chip ID\n4 erase whole chip (use with caution. Takes a long time, up to ~1min)\n5 power down chip (to test current consumption)\n6 power up chip (to test current consumption)\n7 read data from chip\n8 write multiple pages (>256 byte)\n9 erase multiple blocks (64kb per block)\n");
		USART1_flush();
		char strbuf[30];
		uint32_t address;
		uint32_t length;

		switch(USART1_receive()){

		case '1': //erase block(64kb)
			USART1_transmitString("enter 24bit address in HEX format: ");
			USART1_flush();
			USART1_receiveString(strbuf, sizeof(strbuf));
			address = strtol(strbuf, NULL, 16);
			USART1_transmitString(strbuf);
			if( (address < 0) || (address > 0xFFFFFF) ){
				USART1_transmitString("\nERROR: not a valid 24bit address");
				break;
			}
			USART1_transmitString("\nerasing block...");
			block_erase_64KB_W25Q64JV(address);
			USART1_transmitString("\nfinished erasing block.");
			break;

		case '2': //write one page(max. 256kB)
			USART1_transmitString("enter 24bit address in HEX format: ");
			USART1_flush();
			USART1_receiveString(strbuf, sizeof(strbuf));
			address = strtol(strbuf, NULL, 16);
			USART1_transmitString(strbuf);
			if( (address < 0) || (address > 0xFFFFFF) ){
				USART1_transmitString("\nERROR: not a valid 24bit address");
				break;
			}
			USART1_transmitString("send data now");
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
			// wait for first data byte and write it
			SPI_transmit(USART1_receive());
			uint32_t timeout = sysTick_Time;
			// if after a timeout of 500ms no data is available, stop reception
			while(1){
				// wait until the USART data register is not empty and check for timeout
				while (!((USART1->SR) & USART_SR_RXNE) && ((sysTick_Time-timeout) < 500));
				if((sysTick_Time-timeout) >= 500) break;
				// get one byte from USART and pass it on via SPI
				SPI_transmit(((USART1->DR) & 0xFF));
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
			USART1_transmitString("\n writing finished.");
			break;

		case '3':
			USART1_transmitString("unique chip ID is (HEX format): ");
			itoa(get_unique_ID_W25Q64JV(), strbuf, 16);
			USART1_transmitString(strbuf);
			break;

		case '4':
			USART1_transmitString("erasing whole chip...");
			erase_chip_W25Q64JV();
			USART1_transmitString("erasing finished.");
			break;

		case '5':
			power_down_W25Q64JV();
			USART1_transmitString("chip is now powered down");
			break;

		case '6':
			power_up_W25Q64JV();
			USART1_transmitString("chip is now powered up");
			break;

		case '7':
			USART1_transmitString("enter 24bit address in HEX format: ");
			USART1_flush();
			USART1_receiveString(strbuf, sizeof(strbuf));
			address = strtol(strbuf, NULL, 16);
			USART1_transmitString(strbuf);
			if( (address < 0) || (address > 0xFFFFFF) ){
				USART1_transmitString("\nERROR: not a valid 24bit address");
				break;
			}
			USART1_transmitString("\nenter amount of bytes to be read in decimal format: ");
			USART1_receiveString(strbuf, sizeof(strbuf));
			length = strtol(strbuf, NULL, 10);
			USART1_transmitString(itoa(length, strbuf, 10));
			if( length <= 0 ){
				USART1_transmitString("\nERROR: length must be >0");
				break;
			}
			USART1_transmitString("OK. Data transmission will start in 5 seconds. get ready!");
			delay(5000);
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
				USART1_transmit(SPI_transmit(0xFF));
			}
			// CS high, transmission finished
			CS_HIGH();
			break;

		case '8': //write more than one page
			USART1_transmitString("enter 24bit address in HEX format: ");
			USART1_flush();
			USART1_receiveString(strbuf, sizeof(strbuf));
			address = strtol(strbuf, NULL, 16);
			USART1_transmitString(strbuf);
			if( (address < 0) || (address > 0xFFFFFF) ){
				USART1_transmitString("\nERROR: not a valid 24bit address");
				break;
			}
			USART1_transmitString("\nenter amount of bytes to be written in decimal format: ");
			USART1_receiveString(strbuf, sizeof(strbuf));
			length = strtol(strbuf, NULL, 10);
			USART1_transmitString(itoa(length, strbuf, 10));
			if( length <= 0 ){
				USART1_transmitString("\nERROR: length must be >0");
				break;
			}
			USART1_transmitString("\nsend data now!");
			uint32_t num_pages = length / 256;
			uint32_t rest_page_length = length - 256*num_pages;
			for(uint32_t page_iterator = 0; page_iterator<num_pages; page_iterator++){
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
				SPI_transmit(0);
				for(uint32_t byte_counter = 0; byte_counter<256; byte_counter++){
					SPI_transmit(USART1_receive());
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
				//eventually a small delay is needed here, depends on your µC's speed
				asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
				address += 256;
			}
			uint8_t rest_page_buf[255];
			for(uint8_t x=0; x<rest_page_length; x++){
				rest_page_buf[x] = USART1_receive();
			}
			write_W25Q64JV(address, rest_page_length, rest_page_buf);
			USART1_transmitString("\nwriting finished.");
			break;

		case '9': //erase multiple blocks (64kb per block)
			USART1_transmitString("enter 24bit address in HEX format: ");
			USART1_flush();
			USART1_receiveString(strbuf, sizeof(strbuf));
			address = strtol(strbuf, NULL, 16);
			USART1_transmitString(strbuf);
			if( (address < 0) || (address > 0xFFFFFF) ){
				USART1_transmitString("\nERROR: not a valid 24bit address");
				break;
			}
			USART1_transmitString("\nerasing block...");
			block_erase_64KB_W25Q64JV(address);
			USART1_transmitString("\nfinished erasing block.");
			break;

		default:
			USART1_transmitString("no valid argument. try again");
			break;
		}
	}

}
