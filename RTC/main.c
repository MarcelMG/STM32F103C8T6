/*	example for using the Real Time Clock (RTC)
 *	uses Terminal via USART1
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#include "stm32f1xx.h"
#include "init.h"
#include "rtc.h"
#include "usart1.h"
#include <stdlib.h>
#include <time.h>

//	this interrupt occurs every 1 second
void RTC_IRQHandler()
{
    /*
     * 	do something
     *
     */
	// toggle the onboard LED
	GPIOC->ODR ^= GPIO_ODR_ODR13;
	// clear the interrupt flag
    RTC->CRL &=~RTC_CRL_SECF;
}

int main(void) {
	// Initialize system timer for 1ms ticks
	SysTick_Config(SystemCoreClock / 1000);

	// enable GPIO port C for the onboard LED
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    // set pin PC13 (connected to the onboard LED) as output
    GPIOC->CRH &=~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);
    GPIOC->CRH |= GPIO_CRH_MODE13;

    init_USART1();
	USART1_transmitString("initRTC...\n");
	// enable and initialize the RTC
	init_RTC();
	// let the user set the clock via terminal
	USART1_transmitString("enter time in UNIX UTC format (seconds since epoch):\n");
	char time_str[20];
	USART1_receiveString(time_str, 12 );
	time_t current_time = strtoul(time_str, NULL, 10);
	// set the time
	set_RTC(current_time);
	USART1_transmitString("OK\n");

	while (1) {
		// display the current time
		current_time = read_RTC();
		USART1_transmitString("rtc time is: ");
		USART1_transmitString(ctime(&current_time));
		USART1_transmit('\n');
		delay(500);
	}
}


