/*
	SPOKE
    
    file: radio.h
*/



#ifndef RADIO_HEADER
#define RADIO_HEADER

#include "config.h"



void rf_init(void);
uint8_t rf_tx_packet(void);
uint8_t rf_start_rx(void);
void rf_workaround_15_3(void);
void rf_get_rx_packet(void);
void rf_set_standby_xosc(void);
uint8_t rf_get_status(void);
uint16_t rf_get_irq_status(void);
void rf_clear_irq(void);
int8_t rf_get_last_rssi(void);
uint8_t *get_air_packet_tx(void);
uint8_t *get_air_packet_rx(void);



#ifdef FREQ_BAND_433
//CH1 - 433.175 MHz
//CH2 - 433.375 MHz
//CH3 - 433.575 MHz
//CH4 - 433.775 MHz
//CH5 - 433.975 MHz
//CH6 - 434.175 MHz
//CH7 - 434.375 MHz
//CH8 - 434.575 MHz
#define BASE_CHANNEL_FREQUENCY 			(432975000)	// base freq or ch0, not used actually
#define CHANNEL_FREQUENCY_STEP			(200000)
#endif

#ifdef FREQ_BAND_868
//CH1 - 865.100 MHz
//CH2 - 865.300 MHz
//CH3 - 865.500 MHz
//CH4 - 865.700 MHz
//CH5 - 865.900 MHz
//CH6 - 866.100 MHz
//CH7 - 866.300 MHz
//CH8 - 866.500 MHz
//CH9 - 866.700 MHz
//CH10 - 866.900 MHz
//CH11 - 867.100 MHz
//CH12 - 867.300 MHz
//CH13 - 867.500 MHz
//CH14 - 867.700 MHz
//CH15 - 867.900 MHz
//CH16 - 868.100 MHz
//CH17 - 868.300 MHz
//CH18 - 868.500 MHz
//CH19 - 868.700 MHz
//CH20 - 868.900 MHz
//CH21 - 869.100 MHz
#define BASE_CHANNEL_FREQUENCY 			(864900000)	// base freq or ch0, not used actually
#define CHANNEL_FREQUENCY_STEP			(200000)
#endif



#endif /*RADIO_HEADER*/
