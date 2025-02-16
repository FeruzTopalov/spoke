/*
    SPOKE

    file: settings.c
*/

#include "stm32f10x.h"
#include "config.h"
#include "main.h"
#include "settings.h"
#include "memory.h"
#include "gpio.h"
#include "service.h"
#include "lcd.h"
#include "gps.h"
#include "timer.h"



void settings_interactive_save_default(void);



#define UPDATE_INTERVAL_10S_VALUE		(10)
#define UPDATE_INTERVAL_30S_VALUE		(30)
#define UPDATE_INTERVAL_60S_VALUE		(60)

#define UPDATE_INTERVAL_VALUES_ARRAY 	{ 	UPDATE_INTERVAL_10S_VALUE, 	\
											UPDATE_INTERVAL_30S_VALUE, 	\
											UPDATE_INTERVAL_60S_VALUE	}



#define TX_POWER_NEG9DBM_VALUE   	(-9)
#define TX_POWER_POS0DBM_VALUE   	(0)
#define TX_POWER_POS10DBM_VALUE   	(10)
#define TX_POWER_POS22DBM_VALUE  	(22)

#define TX_POWER_VALUES_ARRAY 		{ 	TX_POWER_NEG9DBM_VALUE, 	\
										TX_POWER_POS0DBM_VALUE, 	\
										TX_POWER_POS10DBM_VALUE, 	\
										TX_POWER_POS22DBM_VALUE		}



#define GPS_BAUD_4800_VALUE			(4800)
#define GPS_BAUD_9600_VALUE			(9600)
#define GPS_BAUD_38400_VALUE		(38400)
#define GPS_BAUD_115200_VALUE		(115200)

#define GPS_BAUD_VALUES_ARRAY		{	GPS_BAUD_4800_VALUE,	\
										GPS_BAUD_9600_VALUE,	\
										GPS_BAUD_38400_VALUE,	\
										GPS_BAUD_115200_VALUE 	}


//positions:
#define SETTINGS_INIT_FLAG_POS          	(0)
#define SETTINGS_DEVICE_NUMBER_POS      	(1)
#define SETTINGS_DEVICES_ON_AIR_POS			(2)
#define SETTINGS_DEVICE_ID_POS          	(3)
#define SETTINGS_UPDATE_INTERVAL_POS        (4)
#define SETTINGS_FREQ_CHANNEL_POS       	(5)
#define SETTINGS_TX_POWER_POS           	(6)
#define SETTINGS_GPS_BAUD_POS           	(7)
#define SETTINGS_TIMEOUT_THRESHOLD_POS   	(8)
#define SETTINGS_FENCE_THRESHOLD_POS   		(9)
#define SETTINGS_TIME_ZONE_DIR_POS			(10)
#define SETTINGS_TIME_ZONE_HOUR_POS			(11)
#define SETTINGS_TIME_ZONE_MINUTE_POS		(12)
#define SETTINGS_MAGN_OFFSET_X_POS			(13)
#define SETTINGS_MAGN_OFFSET_Y_POS			(14)
#define SETTINGS_MAGN_SCALE_X_POS			(15)
#define SETTINGS_MAGN_SCALE_Y_POS			(17)

//default values:
#define SETTINGS_INIT_FLAG_DEFAULT      	(0xAA)
#define SETTINGS_DEVICE_NUMBER_DEFAULT  	(DEVICE_NUMBER_FIRST)
#define SETTINGS_DEVICES_ON_AIR_DEFAULT		(DEVICE_NUMBER_LAST)
#define SETTINGS_DEVICE_ID_DEFAULT 			(DEVICE_ID_FIRST_SYMBOL)
#define SETTINGS_UPDATE_INTERVAL_DEFAULT    (UPDATE_INTERVAL_10S_SETTING)
#define SETTINGS_FREQ_CHANNEL_DEFAULT   	(FREQ_CHANNEL_FIRST)
#define SETTINGS_TX_POWER_DEFAULT       	(TX_POWER_POS10DBM_SETTING)
#define SETTINGS_GPS_BAUD_DEFAULT          	(GPS_BAUD_9600_SETTING)
#define SETTINGS_TIMEOUT_THRESHOLD_DEFAULT  (60)
#define SETTINGS_FENCE_THRESHOLD_DEFAULT  	(100)
#define SETTINGS_TIME_ZONE_DIR_DEFAULT		(1)
#define SETTINGS_TIME_ZONE_HOUR_DEFAULT		(0)
#define SETTINGS_TIME_ZONE_MINUTE_DEFAULT	(0)
#define SETTINGS_MAGN_OFFSET_X_DEFAULT		(0)
#define SETTINGS_MAGN_OFFSET_Y_DEFAULT		(0)
#define SETTINGS_MAGN_SCALE_XM_DEFAULT		(0x3f80)	//float 1.0
#define SETTINGS_MAGN_SCALE_XL_DEFAULT		(0x0000)
#define SETTINGS_MAGN_SCALE_YM_DEFAULT		(0x3f80)	//float 1.0
#define SETTINGS_MAGN_SCALE_YL_DEFAULT		(0x0000)

//settings size
#define SETTINGS_SIZE						(19) //half-words



uint16_t settings_array[SETTINGS_SIZE];
struct settings_struct settings;

uint8_t update_interval_values[] = UPDATE_INTERVAL_VALUES_ARRAY;
int8_t tx_power_values[] = TX_POWER_VALUES_ARRAY;
uint32_t gps_baud_values[] = GPS_BAUD_VALUES_ARRAY;



uint8_t *get_update_interval_values(void)
{
	return &update_interval_values[0];
}



int8_t *get_tx_power_values(void)
{
	return &tx_power_values[0];
}



uint32_t *get_gps_baud_values(void)
{
	return &gps_baud_values[0];
}



struct settings_struct *get_settings(void)
{
	return &settings;
}



void settings_load(void)
{
	read_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], 1);

    if (settings_array[SETTINGS_INIT_FLAG_POS] != SETTINGS_INIT_FLAG_DEFAULT)     //if first power-up or FLASH had been erased
    {
        settings_save_default();
    }
    else if (!((GPIOB->IDR) & GPIO_IDR_IDR3) && ((GPIOB->IDR) & GPIO_IDR_IDR4))	//OK released, ESC pressed
    {
    	settings_interactive_save_default();
    }

    //read from flash
    read_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);
    
    //load settings to struct
    settings.device_number = 					settings_array[SETTINGS_DEVICE_NUMBER_POS];
    settings.devices_on_air = 					settings_array[SETTINGS_DEVICES_ON_AIR_POS];
    settings.device_id = 						settings_array[SETTINGS_DEVICE_ID_POS];
    settings.freq_channel = 					settings_array[SETTINGS_FREQ_CHANNEL_POS];
    settings.tx_power_opt = 					settings_array[SETTINGS_TX_POWER_POS];
    settings.update_interval_opt = 				settings_array[SETTINGS_UPDATE_INTERVAL_POS];
    settings.gps_baud_opt =						settings_array[SETTINGS_GPS_BAUD_POS];
    settings.timeout_threshold = 				settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS];
    settings.fence_threshold = 					settings_array[SETTINGS_FENCE_THRESHOLD_POS];
    settings.time_zone_dir = 					settings_array[SETTINGS_TIME_ZONE_DIR_POS];
    settings.time_zone_hour = 					settings_array[SETTINGS_TIME_ZONE_HOUR_POS];
    settings.time_zone_minute = 				settings_array[SETTINGS_TIME_ZONE_MINUTE_POS];
    settings.magn_offset_x =					settings_array[SETTINGS_MAGN_OFFSET_X_POS];
    settings.magn_offset_y =					settings_array[SETTINGS_MAGN_OFFSET_Y_POS];
    settings.magn_scale_x.as_array[0] = 		settings_array[SETTINGS_MAGN_SCALE_X_POS];
    settings.magn_scale_x.as_array[1] = 		settings_array[SETTINGS_MAGN_SCALE_X_POS + 1];
    settings.magn_scale_y.as_array[0] = 		settings_array[SETTINGS_MAGN_SCALE_Y_POS];
    settings.magn_scale_y.as_array[1] = 		settings_array[SETTINGS_MAGN_SCALE_Y_POS + 1];
}



void settings_interactive_save_default(void)
{
	reload_watchdog();	//to prevent reset

	//print instruction
	while (get_lcd_busy()) {}
	lcd_clear();
	lcd_print(0, 0, "DEFAULT SETTINGS");
	lcd_print(2, 3, "OK - Reset");
	lcd_print(3, 2, "ESC - Cancel");
	lcd_update();

	while (!((GPIOB->IDR) & GPIO_IDR_IDR3) || !((GPIOB->IDR) & GPIO_IDR_IDR4)) {}		//wait for user to release OK or ESC after entering settings reset routine
	delay_cyc(100000);

    while (1)	//wait for user's decision
    {
    	reload_watchdog();	//to prevent reset

    	if (!((GPIOB->IDR) & GPIO_IDR_IDR3))	//ECS for exit
    	{
    		NVIC_SystemReset();
    	}

    	if (!((GPIOB->IDR) & GPIO_IDR_IDR4))	//OK for reset settings to default
    	{
    		reload_watchdog();	//to prevent reset

    		while (!((GPIOB->IDR) & GPIO_IDR_IDR4)) {}		//wait for user to release OK
    		delay_cyc(100000);

    		while (get_lcd_busy()) {}
    		lcd_clear();
    		lcd_print(0, 0, "DEFAULT SETTINGS");
    		lcd_print(2, 2, "Resetting...");
    		lcd_update();

    		settings_save_default();

    		/* UNCOMMENT if needed
    		reset_to_defaults_gps_receiver();
    		*/

    		delay_cyc(300000);

    		NVIC_SystemReset();
    	}
    }
}



void settings_save_default(void)
{
	//erase
	erase_flash_page(FLASH_POINTS_PAGE);
	erase_flash_page(FLASH_SETTINGS_PAGE);

    //assign default values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		SETTINGS_DEVICE_NUMBER_DEFAULT;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] = 		SETTINGS_DEVICES_ON_AIR_DEFAULT;
    settings_array[SETTINGS_DEVICE_ID_POS] = 			SETTINGS_DEVICE_ID_DEFAULT;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		SETTINGS_FREQ_CHANNEL_DEFAULT;
    settings_array[SETTINGS_TX_POWER_POS] = 			SETTINGS_TX_POWER_DEFAULT;
    settings_array[SETTINGS_UPDATE_INTERVAL_POS] = 		SETTINGS_UPDATE_INTERVAL_DEFAULT;
    settings_array[SETTINGS_GPS_BAUD_POS] = 			SETTINGS_GPS_BAUD_DEFAULT;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	SETTINGS_TIMEOUT_THRESHOLD_DEFAULT;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		SETTINGS_FENCE_THRESHOLD_DEFAULT;
    settings_array[SETTINGS_TIME_ZONE_DIR_POS] = 		SETTINGS_TIME_ZONE_DIR_DEFAULT;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		SETTINGS_TIME_ZONE_HOUR_DEFAULT;
    settings_array[SETTINGS_TIME_ZONE_MINUTE_POS] = 	SETTINGS_TIME_ZONE_MINUTE_DEFAULT;
    settings_array[SETTINGS_MAGN_OFFSET_X_POS] = 		SETTINGS_MAGN_OFFSET_X_DEFAULT;
    settings_array[SETTINGS_MAGN_OFFSET_Y_POS] = 		SETTINGS_MAGN_OFFSET_Y_DEFAULT;
    settings_array[SETTINGS_MAGN_SCALE_X_POS] = 		SETTINGS_MAGN_SCALE_XL_DEFAULT;	//little-endian
    settings_array[SETTINGS_MAGN_SCALE_X_POS + 1] =		SETTINGS_MAGN_SCALE_XM_DEFAULT;
    settings_array[SETTINGS_MAGN_SCALE_Y_POS] = 		SETTINGS_MAGN_SCALE_YL_DEFAULT;
    settings_array[SETTINGS_MAGN_SCALE_Y_POS + 1] = 	SETTINGS_MAGN_SCALE_YM_DEFAULT;

    
    //write to flash
    write_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);
}



void settings_save(struct settings_struct *p_settings)
{
	erase_flash_page(FLASH_SETTINGS_PAGE);

    //assign values
    settings_array[SETTINGS_INIT_FLAG_POS] = 			SETTINGS_INIT_FLAG_DEFAULT;
    settings_array[SETTINGS_DEVICE_NUMBER_POS] = 		p_settings->device_number;
    settings_array[SETTINGS_DEVICES_ON_AIR_POS] =		p_settings->devices_on_air;
    settings_array[SETTINGS_DEVICE_ID_POS] = 			p_settings->device_id;
    settings_array[SETTINGS_FREQ_CHANNEL_POS] = 		p_settings->freq_channel;
    settings_array[SETTINGS_TX_POWER_POS] = 			p_settings->tx_power_opt;
    settings_array[SETTINGS_UPDATE_INTERVAL_POS] = 		p_settings->update_interval_opt;
    settings_array[SETTINGS_GPS_BAUD_POS] =				p_settings->gps_baud_opt;
    settings_array[SETTINGS_TIMEOUT_THRESHOLD_POS] = 	p_settings->timeout_threshold;
    settings_array[SETTINGS_FENCE_THRESHOLD_POS] = 		p_settings->fence_threshold;
    settings_array[SETTINGS_TIME_ZONE_DIR_POS] = 		p_settings->time_zone_dir;
    settings_array[SETTINGS_TIME_ZONE_HOUR_POS] = 		p_settings->time_zone_hour;
    settings_array[SETTINGS_TIME_ZONE_MINUTE_POS] = 	p_settings->time_zone_minute;
    settings_array[SETTINGS_MAGN_OFFSET_X_POS] = 		p_settings->magn_offset_x;
    settings_array[SETTINGS_MAGN_OFFSET_Y_POS] = 		p_settings->magn_offset_y;
    settings_array[SETTINGS_MAGN_SCALE_X_POS] = 		p_settings->magn_scale_x.as_array[0];
    settings_array[SETTINGS_MAGN_SCALE_X_POS + 1] = 	p_settings->magn_scale_x.as_array[1];
    settings_array[SETTINGS_MAGN_SCALE_Y_POS] = 		p_settings->magn_scale_y.as_array[0];
    settings_array[SETTINGS_MAGN_SCALE_Y_POS + 1] = 	p_settings->magn_scale_y.as_array[1];
    
    //write to flash
    write_flash_page(FLASH_SETTINGS_PAGE, &settings_array[0], SETTINGS_SIZE);
}
