/*
	SPOKE
    
    file: radio.h
*/



#ifndef RADIO_HEADER
#define RADIO_HEADER



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



#endif /*RADIO_HEADER*/
