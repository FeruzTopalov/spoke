/*
	SPOKE
    
    file: points.c
*/


#include "stm32f10x.h"
#include "config.h"
#include "points.h"
#include "memory.h"
#include "lrns.h"
#include "lcd.h"



#define MEMORY_POINT_SIZE	(9)	//9 bytes include: 1 flag, 4 lat; 4 lon.
#define MEMORY_POINTS_SIZE	(MEMORY_POINTS_TOT * MEMORY_POINT_SIZE)

#define MEMORY_POINT_FLAG		(0xAA)	//if a mem point exists then it's flag variable is 0xAA
#define MEMORY_POINT_FLAG_POS	(0)
#define MEMORY_POINT_LAT_POS	(1)
#define MEMORY_POINT_LON_POS	(5)

#define MEM_POINT_NAME_LEN 	(4)			//max len of any point name
#define MEM_POINT_0_NAME   	("HOME")
#define MEM_POINT_1_NAME   	("CAR")
#define MEM_POINT_2_NAME   	("FLAG")
#define MEM_POINT_3_NAME  	("PIN")

#define MEM_POINTS_NAMES_ARRAY 		{ 	{MEM_POINT_0_NAME}, 	\
										{MEM_POINT_1_NAME}, 	\
										{MEM_POINT_2_NAME}, 	\
										{MEM_POINT_3_NAME}	}



struct devices_struct **pp_devices;
uint16_t raw_points_array[MEMORY_POINTS_SIZE]; //uint16 is used because the FLASH organization; actually it is used to carry uint8 data
char memory_points_names_values[MEMORY_POINTS_TOT][MEM_POINT_NAME_LEN + 1] = MEM_POINTS_NAMES_ARRAY;



void init_memory_points(void)
{
	pp_devices = get_devices();
	load_memory_points();
}



void load_memory_points(void)	// FLASH -> buffer array -> devices struct
{
	read_flash_page(FLASH_POINTS_PAGE, &raw_points_array[0], MEMORY_POINTS_SIZE);

	for (uint8_t point = 0; point < MEMORY_POINTS_TOT; point++)
	{
		uint8_t point_start_index = point * MEMORY_POINT_SIZE;
		uint8_t point_device_number = point + MEMORY_POINT_FIRST;

		pp_devices[point_device_number]->device_id = point + SYMB16_HOME; 			//pre-load icons for navigation screen (does not matter point exist or not); refer to file: ssd1306_font16x16.c

		if (raw_points_array[point_start_index + MEMORY_POINT_FLAG_POS] == MEMORY_POINT_FLAG)
		{
			pp_devices[point_device_number]->exist_flag = 1;
			pp_devices[point_device_number]->memory_point_flag = 1;


			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
			{
				pp_devices[point_device_number]->latitude.as_array[b] = raw_points_array[point_start_index + MEMORY_POINT_LAT_POS + b];
				pp_devices[point_device_number]->longitude.as_array[b] = raw_points_array[point_start_index + MEMORY_POINT_LON_POS + b];

			}
		}
		else
		{
			pp_devices[point_device_number]->exist_flag = 0;
			pp_devices[point_device_number]->memory_point_flag = 1;
		}
	}
}



void store_memory_points(void)	// devices struct -> buffer array -> FLASH (pre-erased)
{
	erase_flash_page(FLASH_POINTS_PAGE);

	for (uint8_t point = 0; point < MEMORY_POINTS_TOT; point++)
	{
		uint8_t point_start_index = point * MEMORY_POINT_SIZE;
		uint8_t point_device_number = point + MEMORY_POINT_FIRST;

		if (pp_devices[point_device_number]->exist_flag == 1)
		{
			raw_points_array[point_start_index + MEMORY_POINT_FLAG_POS] = MEMORY_POINT_FLAG;

			for (uint8_t b = 0; b < 4; b++)	//copy lat and lon coordinates
			{
				raw_points_array[point_start_index + MEMORY_POINT_LAT_POS + b] = pp_devices[point_device_number]->latitude.as_array[b];
				raw_points_array[point_start_index + MEMORY_POINT_LON_POS + b] = pp_devices[point_device_number]->longitude.as_array[b];
				//todo: add altitude save
				//todo: add date/time save
			}
		}
		else
		{
			raw_points_array[point_start_index + MEMORY_POINT_FLAG_POS] = 0;
		}
	}

	write_flash_page(FLASH_POINTS_PAGE, &raw_points_array[0], MEMORY_POINTS_SIZE);
}



char *get_memory_point_name(uint8_t point_number)		//point_number from MEMORY_POINT_FIRST to MEMORY_POINT_LAST
{
	return &memory_points_names_values[point_number - MEMORY_POINT_FIRST][0];
}
