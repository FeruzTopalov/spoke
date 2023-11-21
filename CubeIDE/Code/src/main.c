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
uint32_t pps_relative_counter = 0;
uint8_t time_slot_timer_ovf = 0;
uint8_t time_slot = 0;

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

    lcd_print_only(0, 0, "uart..", 0);
    uart1_init();
    uart3_dma_init();

    lcd_print_only(0, 0, "settings..", 0);
    settings_load();

    lcd_print_only(0, 0, "timers..", 0);
    timers_init();

    lcd_print_only(0, 0, "adc..", 0);
    adc_init();
    adc_start_bat_voltage_reading();

    lcd_print_only(0, 0, "i2c..", 0);
    i2c_init();

    lcd_print_only(0, 0, "radio..", 0);
    rf_init();

    lcd_print_only(0, 0, "core..", 0);
    init_lrns();

    lcd_print_only(0, 0, "gps..", 0);
    gps_init();

    lcd_print_only(0, 0, "compass..", 0);
    init_compass();

    lcd_print_only(0, 0, "interrupts..", 0);
    ext_int_init();
    enable_buttons_interrupts();

    lcd_print_only(0, 0, "mem points..", 0);
    init_memory_points();

    lcd_print_only(0, 0, "menu..", 0);
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
				if 	(main_flags.pps_synced == 1)		//ready to txrx when pps exists, data is valid and current second divides by send interval without remainder
				{
					if (p_gps_num->status == GPS_DATA_VALID)
					{
						if ((p_gps_num->second % p_update_interval_values[p_settings->update_interval_opt]) == 0)
						{
							fill_air_packet(uptime);
							main_flags.run_frame = 1;
						}
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
            	release_power();	//just turn off for now
            }

            if (!(main_flags.pps_synced)) 	//when no PPS we still need timeout alarming once in a sec (mostly for our device to alarm about no PPS)
            {
            	calc_timeout(uptime);
            	main_flags.do_beep += check_any_alarm_fence_timeout();
            	main_flags.do_beep += is_battery_low();
            }
        }



        //Checks after receiving packets from all devices or after no txrx; performing beep
        if (main_flags.frame_ended == 1)
        {
        	main_flags.frame_ended = 0;
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

    if (main_flags.pps_synced == 1) 	//if last pps status was "sync" then make a beep because we lost PPS
    {
    	make_a_long_beep();
    }

    pps_relative_counter = 0;
    main_flags.pps_synced = 0;
    main_flags.parse_nmea = 1;

}



//GPS PPS interrupt
void EXTI2_IRQHandler(void)
{

	EXTI->PR = EXTI_PR_PR2;		//clear interrupt
	timer1_start();                 //the first thing to do is to start time slot timer right after PPS

	uart3_dma_stop();				//fix the data
	backup_and_clear_uart_buffer();
	uart3_dma_restart();

	pps_absolute_counter++;
	pps_relative_counter++;

	if (pps_relative_counter > PPS_SKIP) //skip first two pps impulses: skip first PPS - ignore previous nmea data; skip second PPS, but fix the nmea data acquired after first PPS
	{
		main_flags.pps_synced = 1;
		main_flags.parse_nmea = 1;
	}

}



//Radio interrupt (PayloadReady or PacketSent)
void EXTI0_IRQHandler(void)
{
    EXTI->PR = EXTI_PR_PR0;         //clear interrupt

	uint16_t current_radio_status = rf_get_irq_status();	//Process the radio interrupt
	rf_clear_irq();		//clear all flags

	if (current_radio_status & IRQ_RX_DONE)	//Packet received
	{
		main_flags.rx_state = 0;

		if (!(current_radio_status & IRQ_CRC_ERROR))	// if no CRC error
		{
			rf_get_rx_packet();
			parse_air_packet(uptime);   //parse air data from another device (which has ended TX in the current time_slot)

			if (get_current_device() == time_slot)
			{
				led_green_on(); //indicate successful rx event only if received device is active in menu, it will be switched off at the next timeslot interrupt
			}
		}
	}
	else if (current_radio_status & IRQ_TX_DONE)		//Packet transmission completed
	{
		main_flags.tx_state = 0;
		led_green_on(); //indicate successful tx event, led will be switched off at the next timeslot interrupt
	}
	else if (current_radio_status & IRQ_RX_TX_TIMEOUT)	//RX timeout only, because TX timeout feature is not used at all
	{
		main_flags.rx_state = 0;
		rf_set_standby_xosc();	//after RX TO it goes to Standby RC mode only (https://forum.lora-developers.semtech.com/t/sx1268-is-it-possible-to-configure-transition-to-stdby-xosc-after-cad-done-rx-timeout/1282)
	}


}



								//	each byte is timer interrupt mask. timeslot_pattern[0] is the initial state and always zero
								//	0 - skip interrupt
								//	1 - do interrupt
								//	2 - stop timer

								//		|NMEA	|Slot 1		|Slot 2		|Slot 3		|Slot 4		|Slot 5		|Processing	|
								//		|-------|-----------|-----------|-----------|-----------|-----------|-----------|
								//   	0	50	100	150	200	250	300	350	400	450	500	550	600	650	700	750	800	850	900	950	1000 ms
const uint8_t timeslot_pattern[] = {	0, 	0, 	1,	0,	0,	1,	0,	0,	1,	0,	0,	1,	0,	0,	1,	0,	0,	1,	0,	0,	0		};

//Time slot interrupt
void TIM1_UP_IRQHandler(void)
{
    TIM1->SR &= ~TIM_SR_UIF;                    //clear interrupt
    led_green_off();		//switch off green led after successful tx/rx event

    time_slot_timer_ovf++;             			//increment ovf counter

    if (timeslot_pattern[time_slot_timer_ovf] == 1)
    {

    	time_slot++;

        if (main_flags.run_frame == 1)
        {
    		if (time_slot >= (p_settings->devices_on_air + 1)) //devices_on_air regulates how many time slots are active
    		{
    			timer1_stop_reload();			//end of the last time slot
    			time_slot_timer_ovf = 0;
    			time_slot = 0;
    			main_flags.run_frame = 0;
    			main_flags.frame_ended = 1;
    		}
    		else
    		{
    			if (time_slot == p_settings->device_number)
    			{
    				if (rf_tx_packet())
    				{
    					main_flags.tx_state = 1;
    				}
    			}
    			else
    			{
    				if (rf_start_rx())
    				{
    					main_flags.rx_state = 1;
    				}
    			}
    		}
        }
        else
        {
    		timer1_stop_reload();
    		time_slot_timer_ovf = 0;
    		time_slot = 0;

    		if (p_gps_num->status == GPS_DATA_VALID)
        	{
        		main_flags.frame_ended = 1; //if gps is valid then calculate rel pos for all devices
        	}
        	else
        	{
        		main_flags.frame_ended = 0;
        	}
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



