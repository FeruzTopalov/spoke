/*
	SPOKE

    file: adc.h
*/



#ifndef ADC_HEADER
#define ADC_HEADER



void adc_init(void);
void adc_check_bat_voltage(void);
void adc_start_bat_voltage_reading(void);
void adc_read_bat_voltage_result(void);
float get_bat_voltage(void);
uint8_t get_battery_level(void);



#define BAT_LVL_FULL 	(4)
#define BAT_LVL_HIGH 	(3)
#define BAT_LVL_HALF 	(2)
#define BAT_LVL_LOW 	(1)
#define BAT_LVL_EMPTY 	(0)



#endif /*ADC_HEADER*/
