/*
    SPOKE
    
    file: main.h
*/



#ifndef MAIN_HEADER
#define MAIN_HEADER


struct main_flags_struct
{
    uint8_t scan_buttons;
    uint8_t parse_nmea;
    uint8_t update_screen;
    uint8_t run_frame;
    uint8_t process_devices;

    uint8_t tx_state;
    uint8_t rx_state;

    uint8_t menu_changed;
    uint8_t frame_ended;
    uint8_t nmea_parsed_only;
    uint8_t pps_synced;
};


#endif /*MAIN_HEADER*/
