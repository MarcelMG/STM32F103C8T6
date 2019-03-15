 /*	written in 2019 by Marcel Meyer-Garcia
 *  see LICENCE.txt
 */

#ifndef W25Q64JV_H_
#define W25Q64JV_H_

#include "stm32f1xx.h"

void init_W25Q64JV();
void power_down_W25Q64JV();
void power_up_W25Q64JV();
void read_W25Q64JV(uint32_t address, uint32_t length, uint8_t* destination_ptr);
void fast_read_W25Q64JV(uint32_t address, uint32_t length, char* destination_ptr);
void write_W25Q64JV(uint32_t address, uint16_t length, uint8_t* source_ptr);
void sector_erase_W25Q64JV(uint32_t address);
void block_erase_32KB_W25Q64JV(uint32_t address);
void block_erase_64KB_W25Q64JV(uint32_t address);
void erase_chip_W25Q64JV();
uint64_t get_unique_ID_W25Q64JV();
void reset_W25Q64JV();
uint8_t get_status_register1();

#endif /* W25Q64JV_H_ */
