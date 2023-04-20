/*
    SPOKE

    file: lrns.h
*/



#ifndef LRNS_HEADER
#define LRNS_HEADER



extern const double deg_to_rad;       //deg to rad multiplyer



void init_lrns(void);
void fill_air_packet(void);
void parse_air_packet(void);
void process_all_devices(void);
void process_current_device(void);
void calc_relative_position(uint8_t another_device);
void toggle_alarm(void);
uint8_t get_alarm_status(void);
struct devices_struct **get_devices(void);



//Struct with all devices info
struct devices_struct
{
	//COMMON
	uint8_t exist_flag;             //does a device exist?
	uint8_t alarm_flag;				//is alarm reported by a device?
    char device_id;  				//a device id, single ASCII symbol


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


    //MISC
    uint8_t memory_point_flag;			//is this device a memory point?
    uint8_t rx_icon;					//rx icon rotator
};



#endif /*LRNS_HEADER*/
