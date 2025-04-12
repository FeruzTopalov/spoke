/*
    SPOKE
    
    file: uart.c
*/

#include <string.h>
#include "stm32f10x.h"
#include "main.h"
#include "uart.h"
#include "gps.h"
#include "lrns.h"
#include "config.h"
#include "service.h"
#include "settings.h"



void console_prepare_timestamp(void);
void console_prepare_nav_data(void);
void console_conv_nav_data_to_base64(void);
void console_combine_timestamp_nav_data(void);



#define TIMESTAMP_SZ	(27)
#define CONS_DATA_SZ	(256)



char gps_uart_buffer[MAX_UART_BUF_LEN];		//raw UART data for GPS
char *backup_buf;							//backup for raw UART data

uint16_t gps_uart_buf_len; 					//dynamically allocated part of the uart_buffer[MAX_UART_BUF_LEN]
uint16_t brr_gps_baud;						//BRR reg value depending on 'GPS baud' setting

uint8_t console_timestamp[TIMESTAMP_SZ];	//for console, timestamp data

uint8_t console_nav_data[CONS_DATA_SZ];		//for console, raw nav data
uint8_t nav_data_len = 0;					//nav data length

uint8_t console_nav_data_base64[CONS_DATA_SZ];//for console, base64 nav data
uint8_t nav_data_base64_len = 0;			//base64 nav data length

uint8_t console_data[CONS_DATA_SZ];			//resulting console data
uint8_t console_data_len = 0;				//console data length

uint8_t console_report_enabled = 1;			//enable send logs via console

struct devices_struct **pp_devices;
struct settings_struct *p_settings;
struct gps_num_struct *p_gps_num;



//init all uart
void uart_init(void)
{
	pp_devices = get_devices();
	p_settings = get_settings();
	p_gps_num = get_gps_num();

	switch (p_settings->gps_baud_opt)
	{
		case GPS_BAUD_9600_SETTING:		//9600 bod; mantissa 19, frac 8
			brr_gps_baud = 0x0138;
			gps_uart_buf_len = 1000;		//960 bytes/s max, fits buffer
			break;

		case GPS_BAUD_38400_SETTING:	//34800 bod; mantissa 4, frac 14
			brr_gps_baud = 0x004E;
			gps_uart_buf_len = 3500;		//3480 bytes/s max, fits buffer
			break;

		case GPS_BAUD_57600_SETTING:	//57600 bod; mantissa 3, frac 4
			brr_gps_baud = 0x0034;
			gps_uart_buf_len = 5790;		//5760 bytes/s max, fits buffer
			break;

		case GPS_BAUD_115200_SETTING:	//115200 bod; mantissa 1, frac 10
			brr_gps_baud = 0x001A;
			gps_uart_buf_len = 5790;		//11520 bytes/s max, does not fit buffer, pray for fit in actual use (otherwise no RF TX/RX will happen because uart overflow will be hitting earlier than pps interrupt)
			break;

		default:						//9600 bod; mantissa 19, frac 8
			brr_gps_baud = 0x0138;
			gps_uart_buf_len = 1000;		//960 bytes/s max, fits buffer
			break;
	}

	uart1_dma_init();
	uart3_dma_init();
}



//Console UART init
void uart1_dma_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   //ENABLE usart clock

    USART1->BRR = 0x001A; 					//115200 bod
    USART1->CR1 |= USART_CR1_RXNEIE;    	//enable rx interrupt
    USART1->CR1 |= USART_CR1_TE;        	//enable tx
    USART1->CR1 |= USART_CR1_RE;        	//enable rx
    USART1->CR1 |= USART_CR1_UE;        	//uart enable

    USART1->CR3 |= USART_CR3_DMAT;          //enable DMA mode USART TX
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock

    DMA1_Channel4->CPAR = (uint32_t)(&(USART1->DR));    		//transfer destination
    DMA1_Channel4->CMAR = (uint32_t)(&console_data[0]);  		//transfer source
    DMA1_Channel4->CNDTR = 1;                					//bytes amount to transmit

    DMA1_Channel4->CCR |= DMA_CCR4_DIR;		//direction mem -> periph
    DMA1_Channel4->CCR |= DMA_CCR4_MINC;    //enable memory increment
    DMA1_Channel4->CCR |= DMA_CCR4_TCIE;    //enable transfer complete interrupt

    NVIC_EnableIRQ(DMA1_Channel4_IRQn);     //enable DMA interrupt
    DMA1->IFCR = DMA_IFCR_CGIF4;            //clear all interrupt flags for DMA channel 4

    NVIC_EnableIRQ(USART1_IRQn);			//enable UART RX interrupt
}



void uart1_dma_start(void)
{
	DMA1_Channel4->CMAR = (uint32_t)(console_data);		//data starts from index 1
	DMA1_Channel4->CNDTR = console_data_len;			//data size in index 0
	DMA1_Channel4->CCR |= DMA_CCR4_EN;      			//enable channel
}



void uart1_dma_stop(void)
{
	DMA1_Channel4->CCR &= ~DMA_CCR4_EN;     //disable channel
}



void uart1_tx_byte(uint8_t tx_data)
{
    while(!(USART1->SR & USART_SR_TXE))     //wait for transmit register empty
    {
    }
    USART1->DR = tx_data;                      //transmit
}



//switch on/off console reports
void toggle_console_reports(uint8_t enabled)
{
	if (enabled == 0)
	{
		console_report_enabled = 0;
	}
	else
	{
		console_report_enabled = 1;
	}
}



//Send all active devices via console to either BLE or just a terminal
void report_to_console(void)
{
	if (console_report_enabled == 1)
	{
		console_prepare_timestamp();			//timestamp in ISO 8601
		console_prepare_nav_data();				//fill buffer with data
		console_conv_nav_data_to_base64();		//convert to base64
		console_combine_timestamp_nav_data();	//combine all together
		uart1_dma_start();						//send using DMA
	}
}



void console_prepare_timestamp(void)
{
	char tmp_buf[10];

	//FORMAT: "2025-03-02T10:45:00+03:00 "
	//FORMAT: "YYYY-MM-DDTHH:MM:SS+hh:mm "

	//clear
	memset(console_timestamp, 0, TIMESTAMP_SZ);

	//YYYY-
	if ((p_gps_num->year_tz == 0) && (p_gps_num->month_tz == 0) && (p_gps_num->day_tz == 0))
	{
		strcpy(console_timestamp, "0000");		//no date received form GPS, use 0000 as year
	}
	else
	{
		itoa32((p_gps_num->year_tz + 2000), &tmp_buf[0]);		//+2000 to year
		strcpy(console_timestamp, tmp_buf);
	}
	strcat(console_timestamp, "-");

	//MM-
	itoa32(p_gps_num->month_tz, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, "-");

	//DDT
	itoa32(p_gps_num->day_tz, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, "T");

	//HH:
	itoa32(p_gps_num->hour_tz, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, ":");

	//MM:
	itoa32(p_gps_num->minute_tz, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, ":");

	//SS
	itoa32(p_gps_num->second, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);

	//+
	if (p_settings->time_zone_dir > 0)
	{
		strcat(console_timestamp, "+");
	}
	else
	{
		strcat(console_timestamp, "-");
	}

	//hh:
	itoa32(p_settings->time_zone_hour, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, ":");

	//mm
	itoa32(p_settings->time_zone_minute, &tmp_buf[0]);
	time_date_add_leading_zero(&tmp_buf[0]);
	strcat(console_timestamp, tmp_buf);
	strcat(console_timestamp, " ");
}



//What we transmit to console
void console_prepare_nav_data(void)
{
	//init
	uint8_t all_flags = 0;
	nav_data_len = 0;

	//clear
	memset(console_nav_data, 0, CONS_DATA_SZ);

	for (uint8_t dev = NAV_OBJECT_FIRST; dev < NAV_OBJECT_LAST + 1; dev++)		//max: 9 objects x 18 bytes = 162 bytes
	{
		if (pp_devices[dev]->exist_flag == 1)	//only existing devices
		{
			console_nav_data[nav_data_len++] = dev;
			console_nav_data[nav_data_len++] = pp_devices[dev]->device_id;

			all_flags = 0;

			if ((p_settings->device_number) == dev)	//set 0s flag if it is you
			{
				all_flags |= 0x01 << 0;
			}
			else
			{
				all_flags |= 0x00 << 0;
			}

			all_flags |= (	((pp_devices[dev]->memory_point_flag) << 1) 		|\
							((pp_devices[dev]->alarm_flag) << 2) 				|\
							((pp_devices[dev]->fence_flag) << 3) 				|\
							((pp_devices[dev]->timeout_flag) << 4) 				|\
							((pp_devices[dev]->lowbat_flag) << 5) 				|\
							((pp_devices[dev]->link_status_flag) << 6) 			);

			console_nav_data[nav_data_len++] = all_flags;
			console_nav_data[nav_data_len++] = pp_devices[dev]->lora_rssi;

			memcpy(&console_nav_data[nav_data_len], &(pp_devices[dev]->timeout), 4);
			nav_data_len += 4;

			memcpy(&console_nav_data[nav_data_len], &(pp_devices[dev]->latitude.as_float), 4);
			nav_data_len += 4;

			memcpy(&console_nav_data[nav_data_len], &(pp_devices[dev]->longitude.as_float), 4);
			nav_data_len += 4;

			memcpy(&console_nav_data[nav_data_len], &(pp_devices[dev]->altitude), 2);
			nav_data_len += 2;
		}

	}
}



void console_conv_nav_data_to_base64(void)
{
	//init
    nav_data_base64_len = 0;

	//clear
	memset(console_nav_data_base64, 0, CONS_DATA_SZ);

    //encode
    nav_data_base64_len = base64_encode(console_nav_data, console_nav_data_base64, nav_data_len);
}



void console_combine_timestamp_nav_data(void)
{
	//init
	console_data_len = 0;

	//clear
	memset(console_data, 0, CONS_DATA_SZ);

	//copy timestamp
	strcpy(console_data, console_timestamp);

	//copy nav data base64
	strcat(console_data, console_nav_data_base64);

	//add cr+lf
	strcat(console_data, "\r\n");

	//calc len
	console_data_len = string_length(console_data);
}



//GPS UART Init
void uart3_dma_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;   //ENABLE usart clock
    
    //see gpio.c for this pin
    //PB10 - USART3 TX (GPS)
    GPIOB->CRH &= ~GPIO_CRH_MODE10_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE10_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF10_0;     //alternate output push-pull
    GPIOB->CRH |= GPIO_CRH_CNF10_1;

    USART3->BRR = brr_gps_baud;             //per settings
    USART3->CR1 |= USART_CR1_TE;            //enable tx
    USART3->CR1 |= USART_CR1_RE;            //enable rx
    USART3->CR1 |= USART_CR1_UE;            //uart enable
    
    USART3->CR3 |= USART_CR3_DMAR;          //enable DMA mode USART RX
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock
    
    DMA1_Channel3->CPAR = (uint32_t)(&(USART3->DR));    //transfer source
    DMA1_Channel3->CMAR = (uint32_t)(&gps_uart_buffer[0]);  //transfer destination
    DMA1_Channel3->CNDTR = gps_uart_buf_len;                //bytes amount to receive
    
    DMA1_Channel3->CCR |= DMA_CCR3_MINC;    //enable memory increment
    DMA1_Channel3->CCR |= DMA_CCR3_TCIE;    //enable transfer complete interrupt
    DMA1_Channel3->CCR |= DMA_CCR3_EN;      //enable channel
    
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);     //enable interrupts
    DMA1->IFCR = DMA_IFCR_CGIF3;            //clear all interrupt flags for DMA channel 3

    backup_buf = get_nmea_buf();
}



//Stop UART DMA
void uart3_dma_stop(void)
{
    DMA1_Channel3->CCR &= ~DMA_CCR3_EN;     //disable channel
}



//Restart UART DMA
void uart3_dma_restart(void)
{
    DMA1_Channel3->CNDTR = gps_uart_buf_len;    //reload bytes amount to receive
    (void)USART3->SR;						//clear ORE bit due to UART overrun occured between DMA operations
    (void)USART3->DR;
    DMA1_Channel3->CCR |= DMA_CCR3_EN;      //enable channel
}



//Backup uart buffer and then clear it
void backup_and_clear_uart_buffer(void)
{
	memcpy(backup_buf, gps_uart_buffer, gps_uart_buf_len);
	memset(gps_uart_buffer, 0, gps_uart_buf_len);
}



void clear_uart_buffer(void)
{
	memset(gps_uart_buffer, 0, gps_uart_buf_len);
}



void uart3_tx_byte(uint8_t tx_data)
{
    while(!(USART3->SR & USART_SR_TXE))     //wait for transmit register empty
    {
    }
    USART3->DR = tx_data;                      //transmit
}



uint16_t get_gps_uart_buf_len(void)
{
	return gps_uart_buf_len;
}
