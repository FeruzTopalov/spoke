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



struct main_flags_struct main_flags = {0};
struct gps_num_struct *p_gps_num;
struct settings_struct *p_settings;


uint8_t button_code = 0;
uint8_t processing_button = 0;

//TIMERS
uint32_t pps_counter = 0;
uint8_t time_slot_timer_ovf = 0;
uint8_t time_slot = 0;

uint8_t *p_update_interval_values;




//PROGRAM
int main(void)
{
    gpio_init();
    manage_power();

led_red_on();
led_green_on();

    settings_load();
    timers_init();
    spi_init();
    uart1_init();
    uart3_dma_init();
    lcd_init();
    rf_init();
    init_lrns();
    gps_init();
    i2c_init();

    //****
    uint8_t res = 0;
    char buf[10];

    res = i2c_poll(0x32); //acc
    itoa32(res, buf);
    lcd_print(0, 0, buf, 0);
    lcd_update();

    res = i2c_poll(0x3C); //mag
    itoa32(res, buf);
    lcd_print(1, 0, buf, 0);
    lcd_update();

    while (1) {}
    //****

    init_menu();
    init_memory_points();
    ext_int_init();
    enable_buttons_interrupts();


    p_settings = get_settings();
    p_gps_num = get_gps_num();
    p_update_interval_values = get_update_interval_values();



    __enable_irq();

make_a_beep();
led_red_off();
led_green_off();



    while (1)
    {
    	//Scan Keys
    	if (main_flags.scan_buttons == 1)
    	{
    		main_flags.scan_buttons = 0;
    		button_code = scan_button(processing_button);

    		if (button_code)
    		{
    			change_menu(button_code);
    			main_flags.menu_changed = 1;
    		}
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
							fill_air_packet();
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



        //Checks after receiving packets from all devices or after no txrx; performing beep
        if (main_flags.frame_ended == 1)
        {
        	main_flags.frame_ended = 0;
        	main_flags.process_devices = 1;
        	main_flags.update_screen = 1;
        }


        if (main_flags.nmea_parsed_only == 1)	//only nmea parsed (like time and date, num of sats) while no pps exists
        {
        	main_flags.nmea_parsed_only = 0;
        	main_flags.update_screen = 1;
        }


		if (main_flags.menu_changed == 1)
		{
			main_flags.menu_changed = 0;
			main_flags.update_screen = 1;

//			if ((p_gps_num->status == GPS_DATA_VALID) && (main_flags.pps_synced == 1)) //prevent dist calc when no valid gps data
//			{
//	        	main_flags.process_device = 1;
//			}
		}


		//rel pos calc only once
    	if (main_flags.process_devices == 1)
		{
        	main_flags.process_devices = 0;
        	process_all_devices();
		}


    	//screen update (only once in a main cycle)
    	if (main_flags.update_screen == 1)
		{
        	main_flags.update_screen = 0;
        	draw_current_menu();
		}



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
    	//make a long beep
    }

    pps_counter = 0;
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

	pps_counter++;

	if (pps_counter > 2) //skip first two pps impulses: skip first PPS - ignore previous nmea data; skip second PPS, but fix the nmea data acquired after first PPS
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

	//todo add rx timeout interrupt
	if (current_radio_status & IRQ_RX_DONE)	//Packet received
	{
		main_flags.rx_state = 0;
		led_green_off();

		if (!(current_radio_status & IRQ_CRC_ERROR))	// if no CRC error
		{
			rf_get_rx_packet();
			parse_air_packet();   //parse air data from another device (which has ended TX in the current time_slot)
		}
	}
	else if (current_radio_status & IRQ_TX_DONE)		//Packet transmission completed
	{
		main_flags.tx_state = 0;
		led_red_off();
	}
	else if (current_radio_status & IRQ_RX_TX_TIMEOUT)	//RX timeout only, because TX timeout feature is not used at all
	{
		main_flags.rx_state = 0;
		led_green_off();
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
    					led_red_on();
    				}
    			}
    			else
    			{
    				if (rf_start_rx())
    				{
    					main_flags.rx_state = 1;
    					led_green_on();
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
	main_flags.scan_buttons = 1;
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
}



//todo: setupt ints priorities
