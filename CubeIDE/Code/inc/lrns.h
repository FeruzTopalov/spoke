/*
    SPOKE

    file: lrns.h
*/



#ifndef LRNS_HEADER
#define LRNS_HEADER



extern const double deg_to_rad;       //deg to rad multiplyer



void init_lrns(void);
void fill_air_packet(uint32_t current_uptime);
void parse_air_packet(uint32_t current_uptime);
void process_all_devices(void);
void calc_relative_position(uint8_t another_device);
void calc_timeout(uint32_t current_uptime);
void calc_fence(void);
uint8_t check_any_alarm_fence_timeout(void);
void toggle_my_alarm(void);
uint8_t get_my_alarm_status(void);
void set_lowbat_flag(uint8_t value);
struct devices_struct **get_devices(void);



//Struct with all devices info
struct devices_struct
{
	//COMMON
    char device_id;  				//a device id, single ASCII symbol
	uint8_t exist_flag;             //does a device exist?
    uint8_t memory_point_flag;		//is this device a memory point?
	uint8_t alarm_flag;				//is alarm reported by a device?
	uint8_t fence_flag;				//is a predefined fence distance reached?
	uint8_t lowbat_flag;			//did the device transmit low battery flag?


    //TIME
    uint32_t timestamp;					//time stamp in seconds since power-up when the last activity was detected
    uint32_t timeout;					//timeout in seconds since last activity (RX of coordinates)
    uint8_t timeout_flag;				//set when predefined timeout occurs
    uint8_t update_countdown;			//update countdown symbol "9" -> "0" for navigation screen

    //DATE (only for memory points)
    uint8_t save_day;
	uint8_t save_month;
	uint8_t save_year;

    //ABSOLUTE COORDINATES
    union
    {
        float as_float;             //latitude in decimal degrees (-90...+90)
        uint8_t as_array[4];
    } latitude;

    union
    {
        float as_float;             //longitude in decimal degrees (-180...+180)
        uint8_t as_array[4];
    } longitude;

    union
    {
        int16_t as_integer;         //altitude in meters, signed
        uint8_t as_array[2];
    } altitude;


    //RELATIVE COORDINATES
    uint32_t distance;          //distance in meters to a device
    uint16_t heading_deg;       //heading to a device, degrees
    float heading_rad;			//heading to a device, radians
    int16_t delta_altitude;		//delta altitude
};



#endif /*LRNS_HEADER*/
