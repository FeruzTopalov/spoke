/*
    SPOKE
    
    file: main.h
*/



#ifndef MAIN_HEADER
#define MAIN_HEADER



#define PPS_SKIP	(2)		//how many first PPS pulses are skipped before switching to active mode



uint32_t get_abs_pps_cntr(void);



struct main_flags_struct
{
    uint8_t buttons_scanned;
    uint8_t parse_nmea;
    uint8_t update_screen;
    uint8_t run_frame;
    uint8_t do_beep;

    uint8_t tx_state;
    uint8_t rx_state;

    uint8_t frame_ended;
    uint8_t nmea_parsed_only;
    uint8_t pps_synced;

    uint8_t process_compass;

    uint8_t tick_1s;
};


#endif /*MAIN_HEADER*/
