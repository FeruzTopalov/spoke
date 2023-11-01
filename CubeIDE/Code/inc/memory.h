/*
    SPOKE

    file: memory.h
*/



#ifndef MEMORY_HEADER
#define MEMORY_HEADER



#define FLASH_POINTS_PAGE	(0x08018800)	//page 62 start
#define FLASH_SETTINGS_PAGE	(0x08018C00) 	//page 63 start



void erase_flash_page(uint32_t page_address);
void write_flash_page(uint32_t start_address, uint16_t data_array[], uint8_t amount);
void read_flash_page(uint32_t start_address, uint16_t data_array[], uint8_t amount);



#endif /*MEMORY_HEADER*/
