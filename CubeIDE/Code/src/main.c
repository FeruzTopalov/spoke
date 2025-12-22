/*
	SPOKE
    
    file: main.c
*/



#include <string.h>
#include "stm32f10x.h"
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "lcd.h"
#include "service.h"
#include "lcd_bitmaps.h"
#include "timer.h"
#include "buttons.h"
#include "menu.h"
#include "memory.h"
#include "settings.h"
#include "lrns.h"
#include "points.h"
#include "uart.h"
#include "gps.h"
#include "radio.h"
#include "sx126x.h"
#include "i2c.h"
#include "compass.h"
#include "sensors.h"
#include "adc.h"
#include "config.h"



void setup_interrupt_priorities(void);



struct main_flags_struct main_flags = {0};
struct gps_num_struct *p_gps_num;
struct settings_struct *p_settings;
struct devices_struct **pp_devices;



uint8_t button_code = 0;
uint8_t processing_button = 0;

//TIMERS
uint32_t uptime_cntr = 0;
uint32_t nmea_overflow_cntr = 0;
uint32_t pps_absolute_cntr = 0;
uint32_t pps_continuous_cntr = 0;
uint32_t lora_tx_cycles_cntr = 0;
uint32_t lora_rx_cycles_cntr = 0;
uint32_t lora_crc_err_cntr = 0;
uint32_t lora_rx_timeouts_cntr = 0;


uint8_t second_modulo = 0;
uint8_t device_tx_second = 0;
uint8_t max_rx_second = 0;


uint8_t *p_update_interval_values;



//PROGRAM
int main(void)
{
    gpio_init(); //for SPI and backlight pins
    manage_power();
    spi_init();	//for LCD operation
    timers_init(); //for LCD PWM backlight
    settings_load(); //for saved backlight setting
    setup_interrupt_priorities();
    __enable_irq();	//for LCD DMA operation
    lcd_init();
    process_pending_save_default(); //if it was requested upon settings_load()

    //start screen
    lcd_bitmap(&startup_screen[0]);
    lcd_update();
    delay_cyc(800000);

    lcd_print_only(0, 0, "uart..");
    uart_init();

    lcd_print_only(0, 0, "adc..");
    adc_init();
    adc_start_bat_voltage_reading();

    lcd_print_only(0, 0, "i2c..");
    i2c_init();

    lcd_print_only(0, 0, "radio..");
    rf_init();

    lcd_print_only(0, 0, "core..");
    init_lrns();

    lcd_print_only(0, 0, "gps..");
    gps_init();

    lcd_print_only(0, 0, "compass..");
    init_compass();

    lcd_print_only(0, 0, "interrupts..");
    ext_int_init();
    enable_buttons_interrupts();

    lcd_print_only(0, 0, "mem points..");
    init_memory_points();

    lcd_print_only(0, 0, "menu..");
    init_menu();



    //initial variables
    p_settings = get_settings();
    p_gps_num = get_gps_num();
    p_update_interval_values = get_update_interval_values();
    pp_devices = get_devices();

    device_tx_second = (2 * (p_settings->device_number - 1));		//time slots each even second
    max_rx_second = (2 * (p_settings->devices_on_air - 1));



    make_a_beep();	//end of device loading
    draw_current_menu();

    while (1)
    {
    	//Scan Keys
    	if (main_flags.buttons_scanned == 1)
    	{
    		main_flags.buttons_scanned = 0;
			change_menu(button_code);
			reset_backlight_counter();
			main_flags.update_screen = 1;
    	}



        //Parse GPS after PPS interrupt or UART DMA overflow
        if (main_flags.parse_nmea == 1)
        {
            main_flags.parse_nmea = 0;

            if (parse_gps() == 1)
            {
				if 	(main_flags.pps_synced == 1)		//ready to txrx when pps exists & data is valid
				{
					if (p_gps_num->status == GPS_DATA_VALID)
					{
						main_flags.start_radio = 1;
					}
				}
				else
				{
					main_flags.nmea_parsed_only = 1; //show results after successful parsing //only if pps does not exist; otherwise screen_update will be set after frame_end
				}
            }
        }



        //After each second
        if (main_flags.tick_1s == 1)
        {
        	main_flags.tick_1s = 0;

        	decrement_backlight_counter();

            adc_check_bat_voltage();
            if (is_battery_critical())
            {
            	lcd_print_only(2, 2, "BATTERY LOW!");
            	delay_cyc(600000);
            	release_power();	//turn off immediately
            }

			calc_timeout(uptime_cntr);
            if (main_flags.pps_synced == 0) 	//when no PPS we still need timeout alarming once in a sec (mostly for our device to alarm about no PPS)
            {
            	main_flags.do_beep += check_any_alarm_fence_timeout();
            	main_flags.do_beep += is_battery_low();
            }
        }



        //Checks after receiving a packet from a device; performing beep
        if (main_flags.process_all == 1)
        {
        	main_flags.process_all = 0;
        	process_all_devices();

        	calc_fence();
        	calc_timeout(uptime_cntr);
        	main_flags.do_beep += check_any_alarm_fence_timeout();
        	main_flags.do_beep += is_battery_low();
        	main_flags.update_screen = 1;

        	report_to_console(); //send fresh devices data to console
        }


        if (main_flags.nmea_parsed_only == 1)	//only nmea parsed (like time and date, num of sats) while no pps exists
        {
        	main_flags.nmea_parsed_only = 0;
        	main_flags.update_screen = 1;

        	report_to_console(); //send fresh devices data to console
        }



    	if (main_flags.process_compass == 1)
		{
    		main_flags.process_compass = 0;

    		if (read_compass())
    		{
    			main_flags.update_screen = 1;
    		}
		}



    	//screen update (only once in a main cycle)
    	if (main_flags.update_screen == 1)
		{
    		if (!get_lcd_busy())
    		{
    			main_flags.update_screen = 0;
    			draw_current_menu();
    		}
		}



		//beep on alarm/fence/timeout/lowbatt
    	if (main_flags.do_beep != 0)
		{
    		main_flags.do_beep = 0;
    		make_a_beep();
		}


    	//Reload watchdog
    	reload_watchdog();

        //Wait for interrupt
        __WFI();



    } //while(1) end
} //main() end



//DMA UART RX overflow
void DMA1_Channel3_IRQHandler(void)
{
	DMA1->IFCR = DMA_IFCR_CGIF3;     //clear all interrupt flags for DMA channel 3

    uart3_dma_stop();
    backup_and_clear_uart_buffer();
    uart3_dma_restart();

    nmea_overflow_cntr++;
    pps_continuous_cntr = 0;
    main_flags.pps_synced = 0;
    main_flags.parse_nmea = 1;

}



//GPS PPS Interrupt
void EXTI2_IRQHandler(void)
{

	EXTI->PR = EXTI_PR_PR2;			//clear interrupt

	uart3_dma_stop();				//drop the previous data; there will be new after this PPS
	clear_uart_buffer();
	uart3_dma_restart();

	pps_continuous_cntr++;
	pps_absolute_cntr++;

	if (pps_continuous_cntr > MIN_CONT_PPS)		//drop all possible "glitch" pps, or several first pps to get it stabilized
	{
		timer1_start_800ms();           //start gps acquire timer
		main_flags.pps_synced = 1;
	}

}



//Radio Interrupt (PayloadReady or PacketSent)
void EXTI0_IRQHandler(void)
{
    EXTI->PR = EXTI_PR_PR0;         //clear interrupt

	uint16_t current_radio_status = rf_get_irq_status();	//Process the radio interrupt
	rf_clear_irq();		//clear all flags

	if (current_radio_status & IRQ_RX_DONE)	//Packet received
	{
		main_flags.rx_state = 0;
		led_green_off();

		rf_workaround_15_3();	//run if rx timeout was used

		if (!(current_radio_status & IRQ_CRC_ERROR))	// if no CRC error
		{
			uint8_t rx_dev = 0;

			rf_get_rx_packet();
			rx_dev = parse_air_packet(uptime_cntr);   //parse air data from another device (which has ended TX in the current time_slot)
			if (rx_dev != NAV_OBJECT_NULL)
			{
				pp_devices[rx_dev]->lora_snr = rf_get_last_snr();	//read and save SNR
			}

			lora_rx_cycles_cntr++;
		}
		else if (current_radio_status & IRQ_CRC_ERROR)
		{
			lora_crc_err_cntr++;
		}
	}
	else if (current_radio_status & IRQ_TX_DONE)		//Packet transmission completed
	{
		main_flags.tx_state = 0;
		lora_tx_cycles_cntr++;
		led_green_off();
	}
	else if (current_radio_status & IRQ_RX_TX_TIMEOUT)	//RX timeout only, because TX timeout feature is not used at all
	{
		main_flags.rx_state = 0;
		lora_rx_timeouts_cntr++;
		led_green_off();
	}
}



//Timer1 GPS Acquire + Radio Run
void TIM1_UP_IRQHandler(void)
{
    TIM1->SR &= ~TIM_SR_UIF;                    //clear interrupt
    timer1_stop_reload();						//stop this timer

    if (timer1_get_intrvl_type() == TIMER1_INTERVAL_TYPE_LONG) //long interval 800 ms ended
    {

    	//parse NMEA, run short timer
    	timer1_start_100ms();

		uart3_dma_stop();					//fix the nmea data after last pps (800 ms ago)
		backup_and_clear_uart_buffer();
		uart3_dma_restart();				//restart for a case when no next PPS occur, then dma ovf will trigger
										//otherwise next PPS will reset dma again before the upcoming nmea data

    	main_flags.parse_nmea = 1;			//ask to parse

    }
    else if (timer1_get_intrvl_type() == TIMER1_INTERVAL_TYPE_SHORT) //short interval 100 ms ended, resulting 800 ms + 100 ms=900 ms after last pps
    {

    	//important: NMEA must be parsed before execution of this code
    	//start radio, run processing of all devices

    	if (main_flags.start_radio == 1)
    	{
    		main_flags.start_radio = 0;

    		//calc a remainder of current second division by update interval
    		second_modulo = p_gps_num->second % p_update_interval_values[p_settings->update_interval_opt];

    		//timeslots are at X0, X2, X4, X6, X8 second; where X is 0, 1, 2, 3, 4, 5 depending on the update interval
    		if (second_modulo == device_tx_second)
    		{
    			//tx
				fill_air_packet(uptime_cntr);
				if (rf_tx_packet())
				{
					main_flags.tx_state = 1;
					led_green_on();
				}
    		}
    		else if ((second_modulo <= max_rx_second)  && ((second_modulo % 2) == 0))
    		{
    			//rx
				if (rf_start_rx())
				{
					main_flags.rx_state = 1;

					if (second_modulo == (2 * (get_current_device() - 1)))	//blink green if going to receive from current navigate-to device
					{
						led_green_on();
					}
				}
    		}

    	    //after all do a re-calculate all
    	    main_flags.process_all = 1;
    	}
    }
}



//Scan buttons interval
void TIM3_IRQHandler(void)
{
	TIM3->SR &= ~TIM_SR_UIF;        //clear gating timer int

	if (main_flags.buttons_scanned == 0)	//if not scanned yet
	{
		button_code = scan_button(processing_button);
		if (button_code)
		{
			main_flags.buttons_scanned = 1;
		}
	}

}



//DOWN/ESC button interrupt
void EXTI3_IRQHandler(void)
{
	disable_buttons_interrupts();
	EXTI->PR = EXTI_PR_PR3;		//clear interrupt
	processing_button = BUTTON_DOWN_ESC_PB3;
	timer3_start();
}



//UP/OK button interrupt
void EXTI4_IRQHandler(void)
{
	disable_buttons_interrupts();
	EXTI->PR = EXTI_PR_PR4;		//clear interrupt
	processing_button = BUTTON_UP_OK_PB4;
	timer3_start();
}



//PWR button interrupt
void EXTI9_5_IRQHandler(void)
{
	disable_buttons_interrupts();
	EXTI->PR = EXTI_PR_PR5;		//clear interrupt
	processing_button = BUTTON_PWR_PB5;
	timer3_start();
}



void EXTI15_10_IRQHandler(void)
{
#ifdef SPLIT_PWM_BUZZER_BACKLIGHT
	if (EXTI->PR & EXTI_PR_PR15)	//Alarm button interrupt
	{
		enable_my_alarm();
		button_code = BTN_NO_ACTION;
		main_flags.buttons_scanned = 1; //fake buttons scanned event to light up backlight and update menu
		EXTI->PR = EXTI_PR_PR15;		//clear interrupt
	}
#endif

	if (EXTI->PR & EXTI_PR_PR13)	//ACC movement interrupt
	{
		set_acc_movement_flag(ACC_MOVEMENT_DETECTED);
		disable_acc_movement_interrupt();				//once movement is detected, switch interrupt off until the beginning of the next update interval
		EXTI->PR = EXTI_PR_PR13;		//clear interrupt
	}
}



//Console RX symbol
void USART1_IRQHandler(void)
{
    uint32_t uart_sr;
    uint8_t uart_rx_data;

    uart_sr = USART1->SR;   //read SR first

    if (uart_sr & USART_SR_RXNE)
    {
        uart_rx_data = USART1->DR;

        if (uart_rx_data == UART_CMD_CONSOLE_START)
        {
        	switch_console_reports(CONSOLE_REPORT_ENABLED);
        }
        else if (uart_rx_data == UART_CMD_CONSOLE_STOP)
        {
        	switch_console_reports(CONSOLE_REPORT_DISABLED);
        }
    }
    else if (uart_sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE))
    {
        (void)USART1->DR;  // clear error flags performing a read SR and read DR sequence
    }
}



//Console UART TX DMA completed
void DMA1_Channel4_IRQHandler(void)
{
	DMA1->IFCR = DMA_IFCR_CGIF4;     //clear all interrupt flags for DMA channel 4
	uart1_dma_stop();
}



//End of beep
void SysTick_Handler(void)
{
	buzzer_pwm_stop();	//pwm
	systick_stop();	//gating
	led_red_off();
}



//Compass update interval
void TIM4_IRQHandler(void)
{
	TIM4->SR &= ~TIM_SR_UIF;        //clear gating timer int

	main_flags.process_compass = 1;
}



//DMA SPI2 TX LCD
void DMA1_Channel5_IRQHandler(void)
{
	DMA1->IFCR = DMA_IFCR_CGIF5;     //clear all interrupt flags for DMA channel 5

	spi2_dma_stop();
	cs_lcd_inactive();
	lcd_continue_update();
}



//Uptime counter (every 1 second)
void RTC_IRQHandler(void)
{
	RTC->CRL &= ~RTC_CRL_SECF;		//Clear interrupt

    uptime_cntr++;
    main_flags.tick_1s = 1;
}



//End of ADC conversion (battery voltage)
void ADC1_2_IRQHandler(void)
{
	//JEOC cleared by SW
	ADC1->SR &= ~ADC_SR_JEOC;

	adc_read_bat_voltage_result();
}



uint32_t get_uptime_cntr(void)
{
	return uptime_cntr;
}



uint32_t get_nmea_overflow_cntr(void)
{
	return nmea_overflow_cntr;
}



uint32_t get_lora_tx_cycles_cntr(void)
{
	return lora_tx_cycles_cntr;
}



uint32_t get_lora_rx_cycles_cntr(void)
{
	return lora_rx_cycles_cntr;
}



uint32_t get_abs_pps_cntr(void)
{
	return pps_absolute_cntr;
}



uint32_t get_cont_pps_cntr(void)
{
	return pps_continuous_cntr;
}



uint32_t get_lora_crc_errors_cntr(void)
{
	return lora_crc_err_cntr;
}



uint32_t get_lora_rx_timeouts_cntr(void)
{
	return lora_rx_timeouts_cntr;
}



void setup_interrupt_priorities(void)
{
	#define GROUPS_8_SUBGROUPS_2	(4)

	NVIC_SetPriorityGrouping(GROUPS_8_SUBGROUPS_2);

	NVIC_SetPriority(EXTI2_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 0, 0));		//GPS PPS Interrupt
	NVIC_SetPriority(TIM1_UP_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 0, 1));		//Timer1 GPS Acquire + Radio Run

	NVIC_SetPriority(EXTI0_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 1, 0));		//Radio Interrupt
	NVIC_SetPriority(DMA1_Channel3_IRQn, 	NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 1, 1));		//DMA GPS UART RX overflow

	NVIC_SetPriority(DMA1_Channel5_IRQn, 	NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 2, 0));		//DMA SPI2 TX LCD
	NVIC_SetPriority(TIM3_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 2, 1));		//Scan buttons interval

	NVIC_SetPriority(EXTI3_IRQn,			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 3, 0));		//DOWN/ESC button interrupt
	NVIC_SetPriority(EXTI4_IRQn,			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 3, 1));		//UP/OK button interrupt

	NVIC_SetPriority(EXTI9_5_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 4, 0));		//PWR button interrupt
	NVIC_SetPriority(TIM4_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 4, 1));		//Compass update interval

	NVIC_SetPriority(RTC_IRQn, 				NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 5, 0));		//Uptime counter (every 1 second)
	NVIC_SetPriority(USART1_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 5, 1));		//Console RX symbol

	NVIC_SetPriority(DMA1_Channel4_IRQn, 	NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 6, 0));		//Console UART TX DMA completed
	NVIC_SetPriority(ADC1_2_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 6, 1));		//End of ADC conversion (battery voltage)

	NVIC_SetPriority(SysTick_IRQn, 			NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 7, 0));		//End of beep
	NVIC_SetPriority(EXTI15_10_IRQn, 		NVIC_EncodePriority(GROUPS_8_SUBGROUPS_2, 7, 1));		//ALARM button or ACC interrupt
}



