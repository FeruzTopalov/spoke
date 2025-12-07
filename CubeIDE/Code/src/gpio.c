/*
    SPOKE
    
    file: gpio.c
*/



#include "stm32f10x.h"
#include "gpio.h"
#include "bit_band.h"
#include "config.h"



//Initialization of all used ports
void gpio_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;         //enable afio clock
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    //Port A
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    
    //PA0 - Passive Buzzer (PWM) Positive line in Differential drive
    //PA0 - Passive Buzzer (PWM) Single line in Sidgle-ended drive
    GPIOA->CRL &= ~GPIO_CRL_MODE0_0;    //output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE0_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF0_0;       //alternate output push-pull
    GPIOA->CRL |= GPIO_CRL_CNF0_1;
    
    //PA1 - Piezo Buzzer (PWM) Negative line in Differential drive
    //PA1 - LCD Backlight (PWM) in PWM mode Backlight
    GPIOA->CRL &= ~GPIO_CRL_MODE1_0;  	//output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE1_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF1_0;    	//alternate output push-pull
    GPIOA->CRL |= GPIO_CRL_CNF1_1;
    
    //PA2 - ADC2 (Battery voltage)
    GPIOA->CRL &= ~GPIO_CRL_MODE2;      //input mode
    GPIOA->CRL &= ~GPIO_CRL_CNF2;       //analog input
    
    //PA3 - RF Reset
    GPIOA->CRL &= ~GPIO_CRL_MODE3_0;   //output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE3_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF3;      //output push-pull
    
    //PA4 - RF CS
    GPIOA->CRL &= ~GPIO_CRL_MODE4_0;   //output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE4_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF4;      //output push-pull

    //PA5 - RF SCK
    GPIOA->CRL &= ~GPIO_CRL_MODE5_0;    //output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE5_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF5_0;     //alternate output push-pull
    GPIOA->CRL |= GPIO_CRL_CNF5_1;

    //PA6 - RF MISO
    GPIOA->CRL &= ~GPIO_CRL_MODE6;      //input mode
    GPIOA->CRL |= GPIO_CRL_CNF6_0;      //floating input
    GPIOA->CRL &= ~GPIO_CRL_CNF6_1;

    //PA7 - RF MOSI
    GPIOA->CRL &= ~GPIO_CRL_MODE7_0;    //output 2 MHz
    GPIOA->CRL |= GPIO_CRL_MODE7_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF7_0;     //alternate output push-pull
    GPIOA->CRL |= GPIO_CRL_CNF7_1;

    //PA8 - LCD D/C
    GPIOA->CRH &= ~GPIO_CRH_MODE8_0;   //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE8_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF8;      //output push-pull
    
    //PA9 - USART1 TX (Console)
    GPIOA->CRH &= ~GPIO_CRH_MODE9_0;    //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE9_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF9_0;     //alternate output push-pull
    GPIOA->CRH |= GPIO_CRH_CNF9_1;

    //PA10 - USART1 RX (Console)
    GPIOA->CRH &= ~GPIO_CRH_MODE10;     //input
    GPIOA->CRH |= GPIO_CRH_CNF10_0;     //alternate input floating
    GPIOA->CRH &= ~GPIO_CRH_CNF10_1;
    GPIOA->ODR |= GPIO_ODR_ODR10;       //pull-up for stability

    //PA11 - RF TX Enable (in HW 1.x or 2.0) or Power Switch Hold (in HW 2.1 and above)
    GPIOA->CRH &= ~GPIO_CRH_MODE11_0;   //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE11_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF11;      //output push-pull
    
    //PA12 - RF RX Enable
    GPIOA->CRH &= ~GPIO_CRH_MODE12_0;   //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE12_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF12;      //output push-pull
    GPIOA->ODR |= GPIO_ODR_ODR12;		//enable RX on power-up

    //PA15 - Power Switch Hold (in HW 1.x or 2.0) or RF TX Enable (in HW 2.1 and above)
    GPIOA->CRH &= ~GPIO_CRH_MODE15_0;   //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE15_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF15;      //output push-pull

    //Port B
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    //PB0 - RF IRQ (rising edge)
    GPIOB->CRL &= ~GPIO_CRL_MODE0;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF0_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF0_1;
    GPIOB->ODR &= ~GPIO_ODR_ODR0;       //pull-down
    
    //PB1 - RF Busy
    GPIOB->CRL &= ~GPIO_CRL_MODE1;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF1_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF1_1;
    GPIOB->ODR &= ~GPIO_ODR_ODR1;       //pull-down

    //PB2 - GPS (PPS interrupt)
    GPIOB->CRL &= ~GPIO_CRL_MODE2;     //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF2_0;    //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF2_1;
    GPIOB->ODR &= ~GPIO_ODR_ODR2;      //pull-down

    //PB3 - Button ESC
    GPIOB->CRL &= ~GPIO_CRL_MODE3;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF3_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF3_1;
    GPIOB->ODR |= GPIO_ODR_ODR3;        //pull-up on

    //PB4 - Button OK
    GPIOB->CRL &= ~GPIO_CRL_MODE4;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF4_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF4_1;
    GPIOB->ODR |= GPIO_ODR_ODR4;        //pull-up on

    //PB5 - Button PWR
    GPIOB->CRL &= ~GPIO_CRL_MODE5;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF5_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF5_1;
    GPIOB->ODR |= GPIO_ODR_ODR5;        //pull-up on

    //PB6 - ACC/MAG SCL
    GPIOB->CRL &= ~GPIO_CRL_MODE6_0;   //output 2 MHz
    GPIOB->CRL |= GPIO_CRL_MODE6_1;
    GPIOB->CRL |= GPIO_CRL_CNF6;       //alternate function open-drain

    //PB7 - ACC/MAG SCL
    GPIOB->CRL &= ~GPIO_CRL_MODE7_0;   //output 2 MHz
    GPIOB->CRL |= GPIO_CRL_MODE7_1;
    GPIOB->CRL |= GPIO_CRL_CNF7;       //alternate function open-drain

    //PB8 - Red LED
    GPIOB->CRH &= ~GPIO_CRH_MODE8_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE8_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF8;       //output push-pull

    //PB9 - Green LED
    GPIOB->CRH &= ~GPIO_CRH_MODE9_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE9_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF9;       //output push-pull
    
    //PB10 - USART3 TX (GPS)
    GPIOB->CRH &= ~GPIO_CRH_MODE10_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE10_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF10;       //output push-pull
    GPIOB->ODR |= GPIO_ODR_ODR10;			//workaround, this sets RX pin of the GPS to high state and prevents RX frame error which takes place when this pin is low for a long time (between gpio and uart inits)

    //PB11 - USART3 RX (GPS)
    GPIOB->CRH &= ~GPIO_CRH_MODE11;      //input
    GPIOB->CRH |= GPIO_CRH_CNF11_0;      //alternate input floating
    GPIOB->CRH &= ~GPIO_CRH_CNF11_1;
    GPIOB->ODR |= GPIO_ODR_ODR11;        //pull-up for stability

    //PB12 - LCD CS
    GPIOB->CRH &= ~GPIO_CRH_MODE12_0;   //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE12_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF12;      //output push-pull
    
    //PB13 - LCD SCK
    GPIOB->CRH &= ~GPIO_CRH_MODE13_0;   //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE13_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF13_0;    //alternate output push-pull
    GPIOB->CRH |= GPIO_CRH_CNF13_1;
    
    //PB14 - LCD Reset
    GPIOB->CRH &= ~GPIO_CRH_MODE14_0;   //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE14_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF14;      //output push-pull
    
    //PB15 - LCD MOSI
    GPIOB->CRH &= ~GPIO_CRH_MODE15_0;   //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE15_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF15_0;    //alternate output push-pull
    GPIOB->CRH |= GPIO_CRH_CNF15_1;

    //Port C
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    //PC13 - ACC/MAG IRQ
    GPIOC->CRH &= ~GPIO_CRH_MODE13;      //input mode
    GPIOC->CRH &= ~GPIO_CRH_CNF13_0;     //input with pull
    GPIOC->CRH |= GPIO_CRH_CNF13_1;
    GPIOC->ODR |= GPIO_ODR_ODR13;        //pull-up

    //PC14 - X4 test point
    GPIOC->CRH &= ~GPIO_CRH_MODE14_0;    //output 2 MHz
    GPIOC->CRH |= GPIO_CRH_MODE14_1;
    GPIOC->CRH &= ~GPIO_CRH_CNF14;       //output push-pull

#ifdef SPLIT_PWM_BUZZER_BACKLIGHT
    //PC15 - Button Alarm
    GPIOC->CRH &= ~GPIO_CRH_MODE15;      //input mode
    GPIOC->CRH &= ~GPIO_CRH_CNF15_0;     //input with pull
    GPIOC->CRH |= GPIO_CRH_CNF15_1;
    GPIOC->ODR |= GPIO_ODR_ODR15;        //pull-up on
#else
    //PC15 - LCD baclkight
    GPIOC->CRH &= ~GPIO_CRH_MODE15_0;    //output 2 MHz
    GPIOC->CRH |= GPIO_CRH_MODE15_1;
    GPIOC->CRH &= ~GPIO_CRH_CNF15;       //output push-pull
#endif

}



//Init external interrupts
void ext_int_init(void)
{
    //PB0 - RF IRQ
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PB;	//exti 0 source is port B
    EXTI->RTSR |= EXTI_RTSR_TR0;				//interrupt 0 on rising edge
    EXTI->IMR |= EXTI_IMR_MR0;					//unmask interrupt 0
    NVIC_EnableIRQ(EXTI0_IRQn);             	//enable interrupt

    //PB2 - GPS PPS interrupt on rising edge
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PB;	//exti 2 source is port B
    EXTI->RTSR |= EXTI_RTSR_TR2;				//interrupt 2 on rising edge
    EXTI->IMR |= EXTI_IMR_MR2;					//unmask interrupt 2
    NVIC_EnableIRQ(EXTI2_IRQn);             	//enable interrupt

    //PB3 - DOWN/ESC button
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;	//exti 3 source is port B
    EXTI->FTSR |= EXTI_FTSR_TR3;				//interrupt 3 on falling edge
    NVIC_EnableIRQ(EXTI3_IRQn);             	//enable interrupt

    //PB4 - UP/OK button
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;	//exti 4 source is port B
    EXTI->FTSR |= EXTI_FTSR_TR4;				//interrupt 4 on falling edge
    NVIC_EnableIRQ(EXTI4_IRQn);             	//enable interrupt

    //PB5 - PWR button
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;   //exti 5 source is port B
    EXTI->FTSR |= EXTI_FTSR_TR5;                //interrupt 5 on falling edge
    NVIC_EnableIRQ(EXTI9_5_IRQn);               //enable interrupt

#ifdef SPLIT_PWM_BUZZER_BACKLIGHT
    //PC15 - Alarm button
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI15_PC;	//exti 15 source is port C
    EXTI->FTSR |= EXTI_FTSR_TR15;				//interrupt 15 on falling edge
    NVIC_EnableIRQ(EXTI15_10_IRQn);             //enable interrupt
#endif

    EXTI->PR = (uint32_t)0x0007FFFF;            //clear all pending interrupts
}



void enable_buttons_interrupts(void)
{
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR3) = 1;		//unmask interrupt 3
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR4) = 1;		//unmask interrupt 4
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR5) = 1;		//unmask interrupt 5
}



void disable_buttons_interrupts(void)
{
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR3) = 0;		//mask interrupt 3
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR4) = 0;		//mask interrupt 4
	BIT_BAND_PERI(EXTI->IMR, EXTI_IMR_MR5) = 0;		//mask interrupt 5
}



void clear_buttons_interrupts(void)
{
	EXTI->PR = EXTI_PR_PR3;		//clear interrupt
	EXTI->PR = EXTI_PR_PR4;		//clear interrupt
	EXTI->PR = EXTI_PR_PR3;		//clear interrupt
}



//X4 high todo: MUX
void x4_high(void)
{
	GPIOC->BSRR = GPIO_BSRR_BS14;
}



//X4 low todo: MUX
void x4_low(void)
{
	GPIOC->BSRR = GPIO_BSRR_BR14;
}



//LCD backlight on
void backlight_lcd_high(void)
{
#ifndef	SPLIT_PWM_BUZZER_BACKLIGHT
	GPIOC->BSRR = GPIO_BSRR_BS15;
#endif
}



//LCD backlight off
void backlight_lcd_low(void)
{
#ifndef	SPLIT_PWM_BUZZER_BACKLIGHT
	GPIOC->BSRR = GPIO_BSRR_BR15;
#endif
}



//Red led on
void led_red_on(void)
{
    GPIOB->BSRR = GPIO_BSRR_BS8;
}



//Red led off
void led_red_off(void)
{
    GPIOB->BSRR = GPIO_BSRR_BR8;
}



//Red green on
void led_green_on(void)
{
    GPIOB->BSRR = GPIO_BSRR_BS9;
}



//Red green off
void led_green_off(void)
{
    GPIOB->BSRR = GPIO_BSRR_BR9;
}



//RF CS active
void cs_rf_active(void)
{
	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}	//wait RF BUSY line goes low
    GPIOA->BSRR = GPIO_BSRR_BR4;
}



//RF CS inactive
void cs_rf_inactive(void)
{
    GPIOA->BSRR = GPIO_BSRR_BS4;
}



//RF RES active
void res_rf_active(void)
{
    GPIOA->BSRR = GPIO_BSRR_BR3;
}



//RF RES inactive
void res_rf_inactive(void)
{
	GPIOA->BSRR = GPIO_BSRR_BS3;
}



//RES LCD active
void res_lcd_active(void)
{
    GPIOB->BSRR = GPIO_BSRR_BR14;
}



//RES LCD inactive
void res_lcd_inactive(void)
{
    GPIOB->BSRR = GPIO_BSRR_BS14;
}



//Data mode LCD
void lcd_data_mode(void)
{
    GPIOA->BSRR = GPIO_BSRR_BS8;
}



//Command mode LCD
void lcd_command_mode(void)
{
    GPIOA->BSRR = GPIO_BSRR_BR8;
}



//CS LCD active
void cs_lcd_active(void)
{
    GPIOB->BSRR = GPIO_BSRR_BR12;
}



//CS LCD inactive
void cs_lcd_inactive(void)
{
    GPIOB->BSRR = GPIO_BSRR_BS12;
}



//Power switch On
void hold_power(void)
{
#ifdef POWER_HOLD_FIX
	GPIOA->BSRR = GPIO_BSRR_BS11;
#else
	GPIOA->BSRR = GPIO_BSRR_BS15;
#endif
}



//Power switch Off
void release_power(void)
{
#ifdef POWER_HOLD_FIX
	GPIOA->BSRR = GPIO_BSRR_BR11;
#else
    GPIOA->BSRR = GPIO_BSRR_BR15;
#endif
}



//RF TX mode
void rf_tx_mode(void)
{
	GPIOA->BSRR = GPIO_BSRR_BR12;	//off RX

#ifdef POWER_HOLD_FIX
	GPIOA->BSRR = GPIO_BSRR_BS15;	//on TX
#else
	GPIOA->BSRR = GPIO_BSRR_BS11;	//on TX
#endif
}



//RF RX mode
void rf_rx_mode(void)
{
#ifdef	POWER_HOLD_FIX
	GPIOA->BSRR = GPIO_BSRR_BR15;	//off TX
#else
	GPIOA->BSRR = GPIO_BSRR_BR11;	//off TX
#endif
	GPIOA->BSRR = GPIO_BSRR_BS12;	//on RX
}
