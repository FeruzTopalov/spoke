/*
	SPOKE
    
    file: radio.h
*/



#ifndef RADIO_HEADER
#define RADIO_HEADER



void rf_init(void);
uint8_t rf_tx_packet(void);
uint8_t rf_get_irq_status(void);
uint8_t rf_start_rx(void);
uint8_t rf_get_status(void);
void rf_flush_fifo(void);
void rf_get_rx_packet(void);
uint8_t *get_air_packet_tx(void);
uint8_t *get_air_packet_rx(void);



#endif /*RADIO_HEADER*/
