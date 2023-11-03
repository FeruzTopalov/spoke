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



const float vrefint = 1.2;				//STM32 internal reference, channel 17
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

    //Enable Vrefint channel 17
    ADC1->CR2 |= ADC_CR2_TSVREFE;

    //Sample time
    ADC1->SMPR2 |= ADC_SMPR2_SMP0_1;    //13.5 cycles
    
    //Set external trigger event to JSWSTART for injected channels
    ADC1->CR2 |= ADC_CR2_JEXTSEL;
    
    //Enable external trigger event for injected channels
    ADC1->CR2 |= ADC_CR2_JEXTTRIG;
    
    //Set injected sequence length to 2
    ADC1->JSQR |= ADC_JSQR_JL_0;

    //Set the first channel in the sequence to ch 17 (Vrefint)
    ADC1->JSQR |= (ADC_JSQR_JSQ3_4 | ADC_JSQR_JSQ3_0);

    //Set the second channel in the sequence to ch 2 (Vbat resistive divider)
    ADC1->JSQR |= ADC_JSQR_JSQ4_1;

    //Enable scan mode
    ADC1->CR1 |= ADC_CR1_SCAN;
    
    //Interrupt at the end of the conversion for injected channels
    ADC1->CR1 |= ADC_CR1_JEOCIE;
    NVIC_EnableIRQ(ADC1_2_IRQn);
    
    //ADC enable
    ADC1->CR2 |= ADC_CR2_ADON;
    
    //Calibration
    delay_cyc(10000);
    ADC1->CR2 |= ADC_CR2_CAL;           //start cal
    while (ADC1->CR2 & ADC_CR2_CAL);    //wait
}



//Start ADC reading
void adc_start_bat_voltage_reading(void)
{
	//Start conversion of injected channels sequence
	ADC1->CR2 |= ADC_CR2_JSWSTART;
}



//Read the ADC conversion result; return 1 if battery low is detected
void adc_read_bat_voltage_result(void)
{
	//Get injected channels values
	uint32_t dr_vref = 0;
	uint32_t dr_vbat = 0;

	dr_vref = ADC1->JDR1;	//used to calculate Vref+ (for cases when the vcc drops below 3.3)
	dr_vbat = ADC1->JDR2;

	//Convert
	bat_voltage = 2 * vrefint * ((float)dr_vbat / (float)dr_vref);     //x2 due to resistive voltage divider before ADC input
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
