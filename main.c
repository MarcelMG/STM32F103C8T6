/*	testing the Winbond W25Q64JV SPI flash memory
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

#include <stdbool.h>
#include "stm32f1xx.h"
#include "init.h"
#include "W25Q64JV.h"
#include "usart1.h"
#include <stdlib.h>



char strbuf[30];

int main(void)
{
	// Initialize system timer for 1µs ticks (else divide by 1e6 for µs ticks)
	SysTick_Config(SystemCoreClock / 1e6);
	init_USART1();
	USART1_transmitString("hello!\n");

	init_W25Q64JV();


	USART1_transmitString("reading unique ID... got: ");
	itoa(get_unique_ID_W25Q64JV(), strbuf, 10);
	USART1_transmitString(strbuf);
	USART1_transmitString("\nerasing block 0...");

	block_erase_64KB_W25Q64JV(0x000000);
	USART1_transmitString("\nerasing complete.\nwriting some data to sector 0...");
	uint8_t some_data[6] = {1, 2, 3, 4, 5, 6};
	write_W25Q64JV(0x000000, 6, some_data);

	USART1_transmitString("\nwriting complete.\nreading back the data...");
	uint8_t our_data[6];
	fast_read_W25Q64JV(0x000000, 6, our_data);
	USART1_transmitString("\nreading complete.\nreceived:\n");
	for(uint8_t i=0; i<5; i++){
		USART1_transmit(*itoa(our_data[i],strbuf,10));
	}
	USART1_transmitString("\ngoodbye!");
	while(1){

	}

}
