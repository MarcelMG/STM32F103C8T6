/*	minimalist library for using the Real Time Clock (RTC)
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */
#include "rtc.h"

// enable and initialize the RTC
void init_RTC(){
	// 	enable power & backup interface clock
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	//	disable backup domain write protection
	PWR->CR |= PWR_CR_DBP;
	//	enable external 32kHz oscillator (LSE)
	RCC->BDCR |= RCC_BDCR_LSEON;
	//	wait until external 32kHz oscillator (LSE) is ready
	while (!(RCC->BDCR & RCC_BDCR_LSERDY));
	// select LSE as RTC clock source
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	//	enable the RTC clock
	RCC->BDCR |= RCC_BDCR_RTCEN;
	// wait until the RTC_CNT, RTC_ALR and RTC_PRL registers are synchronized
	RTC->CRL &=~ RTC_CRL_RSF;
	while( !(RTC->CRL & RTC_CRL_RSF) );
	// wait until the last write operation is finished
	while( !(RTC->CRL & RTC_CRL_RTOFF) );
	// enable second interrupt
	RTC->CRH |= RTC_CRH_SECIE;
	// wait until the last write operation is finished
	while( !(RTC->CRL & RTC_CRL_RTOFF) );
	//	enter the configuration mode to be able to change the RTC_PRL registers
	RTC->CRL |= RTC_CRL_CNF;
	//	set RTC prescaler to get 1 second from the 32.768kHz crystal oscillator
    RTC->PRLL=32767;
    RTC->PRLH=0;
	//	exit the configuration mode
	RTC->CRL &=~ RTC_CRL_CNF;
	// wait until the last write operation is finished
	while( !(RTC->CRL & RTC_CRL_RTOFF) );
    // enable the RTC interrupt
    NVIC_EnableIRQ(RTC_IRQn);
    // globally enable interrupts
    __enable_irq();
}

// read the current counter value of the RTC
uint32_t read_RTC(){
	// wait until the RTC registers are synchronized
	while( !(RTC->CRL & RTC_CRL_RSF) );
	//	read both RTC counter registers repeatedly until we get twice the same value, thus we
	//	can make sure that the register hasn't changed while reading and we got a wrong value
	uint32_t old_val = 0;
	uint32_t new_val = 0;
	// use a timeout of 100ms to prevent getting stuck in the loop in case of an error
	uint32_t timeout = sysTick_Time;
	do
	{
		old_val = new_val;
		new_val = (((uint32_t) RTC->CNTH) << 16) | ((uint32_t)RTC->CNTL);
	}
	while ( (old_val != new_val) && (sysTick_Time < (timeout+100)) );
	return new_val;
}

// set the value of the RTC counter
void set_RTC( uint32_t seconds ){
	// wait until last write operation is finished
	while( !(RTC->CRL & RTC_CRL_RTOFF) );
	//	enter configuration mode to be able to change the RTC_CNTH & RTC_CNTL registers
	RTC->CRL |= RTC_CRL_CNF;
	//	write new value into the RTC counter registers
    RTC->CNTH = (uint16_t)(seconds >> 16);
    RTC->CNTL = (uint16_t)(seconds & 0xFFFF);
	//	exit configuration mode
	RTC->CRL &=~ RTC_CRL_CNF;
	// wait until the last write operation is finished
	while( !(RTC->CRL & RTC_CRL_RTOFF) );
}
