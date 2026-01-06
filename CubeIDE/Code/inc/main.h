/*
    SPOKE
    
    file: main.h
*/



#ifndef MAIN_HEADER
#define MAIN_HEADER



#define MIN_CONT_PPS	(2)



uint32_t get_uptime_cntr(void);
uint32_t get_nmea_overflow_cntr(void);
uint32_t get_lora_tx_cycles_cntr(void);
uint32_t get_lora_rx_cycles_cntr(void);
uint32_t get_abs_pps_cntr(void);
uint32_t get_cont_pps_cntr(void);
uint32_t get_lora_crc_errors_cntr(void);
uint32_t get_lora_rx_timeouts_cntr(void);



struct main_flags_struct
{
    uint8_t buttons_scanned;
    uint8_t parse_nmea;
    uint8_t start_radio;
    uint8_t update_screen;
    uint8_t do_beep;

    uint8_t tx_state;
    uint8_t rx_state;

    uint8_t process_all;
    uint8_t nmea_parsed_only;
    uint8_t pps_synced;

    uint8_t process_compass;

    uint8_t tick_1s;
};


#endif /*MAIN_HEADER*/
