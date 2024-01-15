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



struct main_flags_struct main_flags = {0};
struct gps_num_struct *p_gps_num;
struct settings_struct *p_settings;


uint8_t button_code = 0;
uint8_t processing_button = 0;

//TIMERS
uint32_t uptime = 0;
uint32_t pps_absolute_counter = 0;

uint8_t *p_update_interval_values;



//PROGRAM
int main(void)
{
    gpio_init();
    manage_power();
    spi_init();
    __enable_irq();	//for LCD DMA operation
    lcd_init();

    //start screen
    lcd_bitmap(&startup_screen[0]);
    lcd_update();
    delay_cyc(400000);

    lcd_print_only(0, 0, "uart..");
    uart1_init();
    uart3_dma_init();

    lcd_print_only(0, 0, "settings..");
    settings_load();

    lcd_print_only(0, 0, "timers..");
    timers_init();

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


    make_a_beep();	//end of device loading
    draw_current_menu();


    while (1)
    {
    	//Scan Keys
    	if (main_flags.buttons_scanned == 1)
    	{
    		main_flags.buttons_scanned = 0;
			change_menu(button_code);
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
            adc_check_bat_voltage();

            if (is_battery_critical())
            {
            	release_power();	//todo: just turn off for now
            }

            if (!(main_flags.pps_synced)) 	//when no PPS we still need timeout alarming once in a sec (mostly for our device to alarm about no PPS)
            {
            	calc_timeout(uptime);
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
        	calc_timeout(uptime);
        	main_flags.do_beep += check_any_alarm_fence_timeout();
        	main_flags.do_beep += is_battery_low();
        	main_flags.update_screen = 1;
        }


        if (main_flags.nmea_parsed_only == 1)	//only nmea parsed (like time and date, num of sats) while no pps exists
        {
        	main_flags.nmea_parsed_only = 0;
        	main_flags.update_screen = 1;
        }



    	if (main_flags.process_compass == 1)
		{
    		main_flags.process_compass = 0;
    		if (read_north())		//todo: decide on applicability of this condition
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



        //Wait for interrupt
        //__WFI();



    } //while(1) end
} //main() end



//DMA UART RX overflow
void DMA1_Channel3_IRQHandler(void)
{

	DMA1->IFCR = DMA_IFCR_CGIF3;     //clear all interrupt flags for DMA channel 3

    uart3_dma_stop();
    backup_and_clear_uart_buffer();
    uart3_dma_restart();

    if (main_flags.pps_synced == 1) 	//if last pps status was "sync" then make a beep because we lost PPS
    {
    	make_a_long_beep();
    }

    main_flags.pps_synced = 0;
    main_flags.parse_nmea = 1;

}



//GPS PPS Interrupt
void EXTI2_IRQHandler(void)
{

	EXTI->PR = EXTI_PR_PR2;			//clear interrupt
	timer1_start_800ms();           //the first thing to do is to start gps acquire timer

	uart3_dma_stop();				//drop the previous data; there will be new after this PPS
	clear_uart_buffer();
	uart3_dma_restart();

	pps_absolute_counter++;

	main_flags.pps_synced = 1;

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
			rf_get_rx_packet();
			parse_air_packet(uptime);   //parse air data from another device (which has ended TX in the current time_slot)
		}
	}
	else if (current_radio_status & IRQ_TX_DONE)		//Packet transmission completed
	{
		main_flags.tx_state = 0;
		led_green_off();
	}
	else if (current_radio_status & IRQ_RX_TX_TIMEOUT)	//RX timeout only, because TX timeout feature is not used at all
	{
		main_flags.rx_state = 0;
		led_green_off();
		rf_set_standby_xosc();	//after RX TO it goes to Standby RC mode only (https://forum.lora-developers.semtech.com/t/sx1268-is-it-possible-to-configure-transition-to-stdby-xosc-after-cad-done-rx-timeout/1282)
	}
}



//Timer1 GPS Acquire + Radio Run
void TIM1_UP_IRQHandler(void)
{
    TIM1->SR &= ~TIM_SR_UIF;                    //clear interrupt
    timer1_stop_reload();						//stop this timer

    if (timer1_get_intrvl_type() == 1) //long interval 800 ms ended
    {

    	//parse NMEA, run short timer
    	timer1_start_100ms();

		uart3_dma_stop();					//fix the nmea data after last pps (800 ms ago)
		backup_and_clear_uart_buffer();
		uart3_dma_restart();				//restart for a case when no next PPS occur, then dma ovf will trigger
										//otherwise next PPS will reset dma again before the upcoming nmea data

    	main_flags.parse_nmea = 1;			//ask to parse

    }
    else if (timer1_get_intrvl_type() == 2) //short interval 100 ms ended, resulting 800 ms + 100 ms=900 ms after last pps
    {

    	//important: NMEA must be parsed before execution of this code
    	//start radio, run processing of all devices

    	if (main_flags.start_radio == 1)
    	{
    		main_flags.start_radio = 0;

    		uint8_t sec_modulo = 0;
    		sec_modulo = p_gps_num->second % p_update_interval_values[p_settings->update_interval_opt];

    		if (sec_modulo == (2 * (p_settings->device_number - 1)))	//todo: add to settings as device_tx_second
    		{
    			//tx
				fill_air_packet(uptime);
				if (rf_tx_packet())
				{
					main_flags.tx_state = 1;
					led_green_on();
				}
    		}
    		else if ((sec_modulo <= (2 * (p_settings->devices_on_air - 1)))  && ((sec_modulo % 2) == 0))	//todo: add to settings as max_rx_second
    		{
    			//rx
				if (rf_start_rx())
				{
					main_flags.rx_state = 1;
					if (sec_modulo == (2 * (get_current_device() - 1)))	//blink green if going to receive from current navigate-to device
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



//Console RX symbol
void USART1_IRQHandler(void)
{
    uint8_t rx_data;
    rx_data = USART1->DR;
    uart1_tx_byte(++rx_data);	//simple incremental echo test
}



//End of beep
void SysTick_Handler(void)
{
	timer2_stop();	//pwm
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

    uptime++;
    main_flags.tick_1s = 1;
}



//End of ADC conversion (battery voltage)
void ADC1_2_IRQHandler(void)
{
	//JEOC cleared by SW
	ADC1->SR &= ~ADC_SR_JEOC;

	adc_read_bat_voltage_result();
}



uint32_t get_abs_pps_cntr(void)
{
	return pps_absolute_counter;
}


//todo: setupt ints priorities



