/*
    SPOKE

    file: memory.c
*/

#include "stm32f10x.h"
#include "bit_band.h"
#include "memory.h"



#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)



void erase_flash_page(uint32_t page_address)
{
	while (FLASH->SR & FLASH_SR_BSY);	//check busy

	if (FLASH->SR & FLASH_SR_EOP)	//clear end of operation flag
	{
		FLASH->SR = FLASH_SR_EOP;
	}

	//unlock flash
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	if (!(FLASH->CR & FLASH_CR_LOCK))	//if unlock is successful
	{
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PER) = 1;	//set page erase mode
		FLASH->AR = page_address; //load address of the page to erase
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_STRT) = 1;	//start erase
		while (!(FLASH->SR & FLASH_SR_EOP));	//wait
		FLASH->SR = FLASH_SR_EOP;
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PER) = 0;	//unset erase mode
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_LOCK) = 1;	//lock flash
	}
}



void write_flash_page(uint32_t start_address, uint16_t data_array[], uint8_t amount)
{
	while (FLASH->SR & FLASH_SR_BSY);	//check busy

	if (FLASH->SR & FLASH_SR_EOP)	//clear end of operation flag
	{
		FLASH->SR = FLASH_SR_EOP;
	}

	//unlock flash
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	if (!(FLASH->CR & FLASH_CR_LOCK))	//if unlock is successful
	{
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PG) = 1;	//set page write mode

		for (uint8_t i = 0; i < amount; i++)
		{
			((__IO uint16_t *)start_address)[i] = data_array[i];	//write
			while (!(FLASH->SR & FLASH_SR_EOP));	//wait
			FLASH->SR = FLASH_SR_EOP;
		}

		BIT_BAND_PERI(FLASH->CR, FLASH_CR_PG) = 0;	//unset write mode
		BIT_BAND_PERI(FLASH->CR, FLASH_CR_LOCK) = 1;	//lock flash
	}
}



void read_flash_page(uint32_t start_address, uint16_t data_array[], uint8_t amount)
{
	for (uint8_t i = 0; i < amount; i++)
	{
		data_array[i] = ((__IO uint16_t *)start_address)[i];
	}
}
