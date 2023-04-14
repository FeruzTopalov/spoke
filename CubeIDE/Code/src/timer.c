/*
	SPOKE
    
    file: timer.c
*/

#include "stm32f10x.h"
#include "timer.h"
#include "gpio.h"
#include "bit_band.h"



void timer1_init(void);
void timer1_clock_disable(void);
void timer1_clock_enable(void);
void timer2_init(void);



uint8_t sound_enabled = 1; //status of the beep sound notification



//Init all timers together
void timers_init(void)
{
	timer1_init();
	timer2_init();
}



//Timer1 init (time slot counter)
void timer1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;     //enable timer 1 clock
    TIM1->PSC = (uint16_t)2999;            	// 3MHz/(2999+1)=1kHz
    TIM1->ARR = (uint16_t)49;               // 1kHz/(49+1)=20Hz (50ms)
    TIM1->CR1 |= TIM_CR1_URS;               //only overflow generates interrupt
    TIM1->EGR = TIM_EGR_UG;                 //software update generation
    TIM1->SR &= ~TIM_SR_UIF;                //clear update interrupt
    TIM1->DIER |= TIM_DIER_UIE;             //update interrupt enable

    NVIC_EnableIRQ(TIM1_UP_IRQn);           //enable interrupt
    timer1_clock_disable();
}



void timer1_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) = 0;
}



void timer1_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) = 1;
}



//Timer1 start
void timer1_start(void)
{
	timer1_clock_enable();
    TIM1->CR1 |= TIM_CR1_CEN;               //enable counter
}



//Timer1 stop and reload
void timer1_stop_reload(void)
{
    TIM1->CR1 &= ~TIM_CR1_CEN;              //disable counter
    TIM1->EGR = TIM_EGR_UG;                 //software update generation
    timer1_clock_disable();
}



//Timer 2 init (buttons scan interval)
void timer2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //enable timer clock
	TIM2->PSC = (uint16_t)299;         	// 3MHz/(299+1)=10kHz
	TIM2->ARR = (uint16_t)99;           // 10kHz/(99+1)=100Hz(10ms)
	TIM2->EGR = TIM_EGR_UG;             //software update generation
	TIM2->DIER |= TIM_DIER_UIE;         //update interrupt enable

	NVIC_EnableIRQ(TIM2_IRQn);
}



void timer2_start(void)
{
	BIT_BAND_PERI(TIM2->CR1, TIM_CR1_CEN) = 1;	//start timer
}



void timer2_stop(void)
{
	BIT_BAND_PERI(TIM2->CR1, TIM_CR1_CEN) = 0;	//stop  timer
	TIM2->CNT = 0;					//reset counter
	TIM2->SR &= ~TIM_SR_UIF;        //clear int
}



void toggle_sound(void)
{
	if (sound_enabled == 0)
	{
		sound_enabled = 1;
	}
	else
	{
		sound_enabled = 0;
	}
}



uint8_t get_sound_status(void)
{
	return sound_enabled;
}
