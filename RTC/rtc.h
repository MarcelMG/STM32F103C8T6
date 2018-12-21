/*	minimalist library for using the Real Time Clock (RTC)
 *
 *  written in 2018 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#ifndef RTC_H_
#define RTC_H_

#include "stm32f1xx.h"
#include "init.h"

// enable and initialize the RTC
void init_RTC();
// read the current counter value of the RTC
uint32_t read_RTC();
// set the value of the RTC counter
void set_RTC( uint32_t seconds);

#endif /* RTC_H_ */
