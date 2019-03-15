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









int main(void)
{
	// Initialize system timer for 1µs ticks (else divide by 1e6 for µs ticks)
	SysTick_Config(SystemCoreClock / 1e6);

	while(1){

	}

}
