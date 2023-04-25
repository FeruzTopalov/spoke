/*
	SPOKE
    
    file: radio.h
*/



#ifndef RFM98_HEADER
#define RFM98_HEADER



void rfm98_init(void);
uint8_t rfm98_tx_packet(void);
uint8_t rfm98_get_irq_status(void);
uint8_t rfm98_start_rx(void);
void rfm98_flush_fifo(void);
void rfm98_get_rx_packet(void);
uint8_t *get_air_packet_tx(void);
uint8_t *get_air_packet_rx(void);



#endif /*RFM98_HEADER*/
