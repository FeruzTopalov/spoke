/*
	SPOKE

    file: adc.h
*/



#ifndef ADC_HEADER
#define ADC_HEADER



#define V_BATTERY_CRIT      (3.0)
#define V_BATTERY_MIN       (3.4)
#define V_BATTERY_MAX     	(4.2)



void adc_init(void);
void adc_clock_disable(void);
void adc_clock_enable(void);
void adc_check_bat_voltage(void);
void adc_start_bat_voltage_reading(void);
void adc_read_bat_voltage_result(void);
float get_bat_voltage(void);
uint8_t is_battery_low(void);
uint8_t is_battery_critical(void);
uint8_t get_battery_level(void);



#endif /*ADC_HEADER*/
