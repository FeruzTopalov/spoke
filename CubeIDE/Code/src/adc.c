/*
	SPOKE

    file: adc.h
*/



#include "stm32f10x.h"
#include "adc.h"
#include "service.h"



void adc_start_bat_voltage_reading(void);



#define GET_BAT_VOLTAGE_INTERVAL    (10)



#define V_BATTERY_MIN       (3.0)
#define V_BATTERY_MAX     	(4.2)



const float vref = 3.3;
uint8_t bat_interval_counter = 0;
float bat_voltage;
uint8_t bat_level;



//ADC Init
void adc_init(void)
{
    //ADC prescaller
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;      //div by 8
    
    //ADC clock on
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    
    //Sample time
    ADC1->SMPR2 |= ADC_SMPR2_SMP0_1;    //13.5 cycles
    
    //Number of conversions in regular sequence
    ADC1->SQR1 &= ~ADC_SQR1_L;          //1 conversion
    
    //First channel in regular sequence
    ADC1->SQR3 |= ADC_SQR3_SQ1_1;        //channel #2
    
    //Extermal trigger enable for regular sequence
    ADC1->CR2 |= ADC_CR2_EXTTRIG;
    
    //Event to start regular sequence
    ADC1->CR2 |= ADC_CR2_EXTSEL;        //start by software
    
    //ADC enable
    ADC1->CR2 |= ADC_CR2_ADON;
    
    //Calibration
    delay_cyc(10000);
    ADC1->CR2 |= ADC_CR2_CAL;           //start cal
    while (ADC1->CR2 & ADC_CR2_CAL);    //wait

    //Interrupt at the end of the conversion
    ADC1->CR1 |= ADC_CR1_EOCIE;
    NVIC_EnableIRQ(ADC1_2_IRQn);
}



//Start ADC reading
void adc_start_bat_voltage_reading(void)
{
	//Start conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
}



//Read the ADC conversion result; return 1 if battery low is detected
void adc_read_bat_voltage_result(void)
{
	//Convert
	bat_voltage = 2 * ((ADC1->DR * vref) / 4096);     //x2 due to resistive voltage divider before ADC input
}



//Check battery voltage with predefined interval
void adc_check_bat_voltage(void)
{
	bat_interval_counter++;

    if (bat_interval_counter >= GET_BAT_VOLTAGE_INTERVAL)
    {
    	bat_interval_counter = 0;

    	adc_start_bat_voltage_reading();
    }
}



float get_bat_voltage(void)
{
	return bat_voltage;
}



uint8_t get_battery_level(void)
{
	//scale to 16 levels
	float v_tmp;
	float lvl;
	uint8_t lvl_scaled;
	v_tmp = bat_voltage - V_BATTERY_MIN;
	lvl = v_tmp * (V_BATTERY_MAX - V_BATTERY_MIN);
	lvl_scaled = lvl * 16;

	if (lvl_scaled > 15)
	{
		lvl_scaled = 15;
	}

	return lvl_scaled;
}