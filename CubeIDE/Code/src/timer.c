/*
	SPOKE
    
    file: timer.c
*/

#include "stm32f10x.h"
#include "timer.h"
#include "gpio.h"
#include "bit_band.h"
#include "config.h"



void systick_init(void);
void systick_start(void);
void systick_set_100ms(void);
void systick_set_1000ms(void);
void timer1_init(void);
void timer1_clock_disable(void);
void timer1_clock_enable(void);
void timer2_init(void);
void timer2_clock_disable(void);
void timer2_clock_enable(void);
void timer2_start(void);
void timer2_stop(void);
void buzzer_pwm_start(void);
void timer3_init(void);
void timer3_clock_disable(void);
void timer3_clock_enable(void);
void timer4_init(void);
void timer4_clock_disable(void);
void timer4_clock_enable(void);



uint8_t sound_enabled = 1; //status of the beep sound notification
uint8_t timer1_interval_type = 0;



//Init all timers together
void timers_init(void)
{
	rtc_init();
	systick_init();
	timer1_init();
	timer2_init();
	timer3_init();
	timer4_init();
}



void make_a_beep(void)
{
	if (sound_enabled)
	{
		buzzer_pwm_start();		//pwm
	}

	systick_set_100ms();
	systick_start();	//gating
	led_red_on();
}



void make_a_long_beep(void)
{
	if (sound_enabled)
	{
		buzzer_pwm_start();		//pwm
	}

	systick_set_1000ms();
	systick_start();	//gating
	led_red_on();
}



void init_watchdog(void)
{
	RCC->CSR |= RCC_CSR_LSION;  					//Enable LSI 40 kHz
	while ((RCC->CSR & RCC_CSR_LSIRDY) == 0){}		//Wait for stabilize

	IWDG->KR |= 0x5555;			//Unlock IWDG configuration
	IWDG->PR |= IWDG_PR_PR_2;	//Value 100: divider /64; max timeout 6553.6 ms
	IWDG->KR |= 0xAAAA;			//Reload IWDG
	IWDG->KR |= 0xCCCC;			//Start IWDG
}



void reload_watchdog(void)
{
	IWDG->KR |= 0xAAAA;			//Reload IWDG
}



void rtc_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;				//Enable power interface
	RCC->APB1ENR |= RCC_APB1ENR_BKPEN;				//Enable backup interface
	PWR->CR |= PWR_CR_DBP;							//Disable backup domain write protection. Backup domain write protection should be disabled if HSE/128 is used as RTC clock source
    RCC->BDCR |=  RCC_BDCR_BDRST;					//Reset entire backup domain (reset RTC)
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    RCC->BDCR |= RCC_BDCR_RTCSEL_HSE;				//HSE/128 is RTC clock source (8 MHz / 128 = 62.5 kHz)
	RCC->BDCR |= RCC_BDCR_RTCEN;					//Enable RTC

    RTC->CRL &= ~RTC_CRL_RSF;                 		//Clear the RSF flag
    while (!(RTC->CRL & RTC_CRL_RSF)){};       		//Wait until RSF is set
	while ((RTC->CRL & RTC_CRL_RTOFF) == 0){}		//Wait RTC ready to acquire new command
	RTC->CRL |= RTC_CRL_CNF;						//Enter config mode
	RTC->PRLH = 0;
	RTC->PRLL = 62499;								//(62500Hz-1)
	RTC->CRH |= RTC_CRH_SECIE;						//Enable Interrupt
	RTC->CRL &= ~RTC_CRL_CNF;						//Exit config mode
	while ((RTC->CRL & RTC_CRL_RTOFF) == 0){}

	NVIC_EnableIRQ(RTC_IRQn);
}



//SysTick timer init
void systick_init(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;   //clock source = AHB/8 = 3MHz/8 = 375 kHz
    SysTick->LOAD = (uint32_t)37499;              	//375000Hz/(37499+1)=10Hz
    SysTick->VAL = 0;                               //reset counter value
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;      //enable interrupt
}



//Start buzzer gating timer
void systick_start(void)
{
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}



//Stop buzzer gating timer
void systick_stop(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}



void systick_set_100ms(void)
{
	SysTick->LOAD = (uint32_t)37499;              	//375000Hz/(37499+1)=10Hz
	SysTick->VAL = 0;
}



void systick_set_1000ms(void)
{
	SysTick->LOAD = (uint32_t)374999;              	//375000Hz/(374999+1)=1Hz
	SysTick->VAL = 0;
}



//Timer1 init (time slot counter)
void timer1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;     //enable timer 1 clock
    TIM1->PSC = (uint16_t)2999;            	// 3MHz/(2999+1)=1kHz
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



//Timer1 start 800 ms
void timer1_start_800ms(void)
{
	timer1_clock_enable();
	TIM1->ARR = (uint16_t)799;              // 1kHz/(799+1)=1.25Hz (800ms)
    TIM1->CR1 |= TIM_CR1_CEN;               //enable counter
    timer1_interval_type = TIMER1_INTERVAL_TYPE_LONG;
}



//Timer1 start 100 ms
void timer1_start_100ms(void)
{
	timer1_clock_enable();
	TIM1->ARR = (uint16_t)99;              // 1kHz/(99+1)=10Hz (100ms)
    TIM1->CR1 |= TIM_CR1_CEN;               //enable counter
    timer1_interval_type = TIMER1_INTERVAL_TYPE_SHORT;
}



uint8_t timer1_get_intrvl_type(void)
{
	return timer1_interval_type;
}



//Timer1 stop and reload
void timer1_stop_reload(void)
{
    TIM1->CR1 &= ~TIM_CR1_CEN;              //disable counter
    TIM1->EGR = TIM_EGR_UG;                 //software update generation
    timer1_clock_disable();
}



//Timer 2 init (buzzer and/or backlight pwm timer, the frequency of the "beep" and/or backlight pwm)
void timer2_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //enable timer clock
	TIM2->PSC = (uint16_t)374;         	//3MHz/(374+1)=8kHz
	TIM2->ARR = (uint16_t)3;            //8kHz/(3+1)=2kHz

#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	TIM2->CCR1 = (uint16_t)2;           //duty cycle 2/(3+1)=0.5 (fixed)
	TIM2->CCR2 = (uint16_t)1;           //duty cycle 1/(3+1)=0.25 (configurable by user via settings)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2); //Force low CH1 (PA0, buzzer)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2); //Force low CH2 (PA1, backlight)
#else
	TIM2->CCR1 = (uint16_t)2;           //duty cycle 2/(3+1)=0.5 (fixed)
	TIM2->CCR2 = (uint16_t)2;           //duty cycle 2/(3+1)=0.5 (fixed)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2);    //Force low CH1 (PA0, buzzer p)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2);    //Force low CH2 (PA1, buzzer n)
#endif

	TIM2->CCER |= TIM_CCER_CC1E;   	//CH1 output enable
	TIM2->CCER |= TIM_CCER_CC2E;    //CH2 output enable
	TIM2->CR1 |= TIM_CR1_CEN;   	//enable PWM timer
}



void timer2_clock_disable(void) //todo del
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM2EN) = 0;
}



void timer2_clock_enable(void) //todo del
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM2EN) = 1;
}



void timer2_start(void) //todo del
{
	;
}



void timer2_stop(void) //todo del
{
	;
}



void buzzer_pwm_start(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1); //PWM mode 1 for CH1 (PA0, buzzer)
#else
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);    //PWM mode 1 for CH1 (PA0, buzzer p)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_0);    //PWM mode 2 for CH2 (PA1, buzzer n)
#endif
}



void buzzer_pwm_stop(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2); //Force low CH1 (PA0, buzzer)
#else
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_2);    //Force low CH1 (PA0, buzzer p)
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2);    //Force low CH2 (PA1, buzzer n)
#endif
}



void backlight_pwm_start(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1); //PWM mode 1 for CH2 (PA1, backlight)
#endif
}



void backlight_pwm_stop(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC2M) | (TIM_CCMR1_OC2M_2); //Force low CH2 (PA1, backlight)
#endif
}



//Timer 3 init (buttons scan interval)
void timer3_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //enable timer clock
	TIM3->PSC = (uint16_t)299;         	// 3MHz/(299+1)=10kHz
	TIM3->ARR = (uint16_t)99;           // 10kHz/(99+1)=100Hz(10ms)
	TIM3->EGR = TIM_EGR_UG;             //software update generation
	TIM3->SR &= ~TIM_SR_UIF;        	//clear gating timer int
	TIM3->DIER |= TIM_DIER_UIE;         //update interrupt enable

	NVIC_EnableIRQ(TIM3_IRQn);
	timer3_clock_disable();
}



void timer3_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM3EN) = 0;
}



void timer3_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM3EN) = 1;
}



void timer3_start(void)
{
	timer3_clock_enable();

	BIT_BAND_PERI(TIM3->CR1, TIM_CR1_CEN) = 1;	//start timer
}



void timer3_stop(void)
{
	BIT_BAND_PERI(TIM3->CR1, TIM_CR1_CEN) = 0;	//stop  timer
	TIM3->CNT = 0;					//reset counter
	TIM3->SR &= ~TIM_SR_UIF;        //clear int

	timer3_clock_disable();
}



//Timer 4 init (compass update)
void timer4_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; //enable timer clock
	TIM4->PSC = (uint16_t)299;         	// 3MHz/(299+1)=10kHz
	TIM4->ARR = (uint16_t)999;          // 10kHz/(999+1)=10Hz(100ms)
	TIM4->EGR = TIM_EGR_UG;             //software update generation
	TIM4->SR &= ~TIM_SR_UIF;        	//clear gating timer int
	TIM4->DIER |= TIM_DIER_UIE;         //update interrupt enable

	NVIC_EnableIRQ(TIM4_IRQn);
	timer4_clock_disable();
}



void timer4_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM4EN) = 0;
}



void timer4_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_TIM4EN) = 1;
}



void timer4_start(void)
{
	timer4_clock_enable();

	BIT_BAND_PERI(TIM4->CR1, TIM_CR1_CEN) = 1;	//start timer
}



void timer4_stop(void)
{
	BIT_BAND_PERI(TIM4->CR1, TIM_CR1_CEN) = 0;	//stop  timer
	TIM4->CNT = 0;					//reset counter
	TIM4->SR &= ~TIM_SR_UIF;        //clear int

	timer4_clock_disable();
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
