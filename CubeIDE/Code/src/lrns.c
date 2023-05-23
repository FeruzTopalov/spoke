/*
    SPOKE

    file: lrns.c
*/



#include <string.h>
#include <math.h>
#include "stm32f10x.h"
#include "config.h"
#include "lrns.h"
#include "settings.h"
#include "gps.h"
#include "radio.h"
#include "menu.h"
#include "lcd.h"
#include "gpio.h"



void update_reception_icon(uint8_t rx_device);



const double rad_to_deg = 57.29577951308232;        //rad to deg multiplyer
const double deg_to_rad = 0.0174532925199433;       //deg to rad multiplyer
const double twice_mean_earth_radius = 12742016.0;  // 2 * 6371008 meters
const double pi_div_by_1 = 3.141592653589793;      // pi / 1
const double pi_mul_by_2 = 6.283185307179586;      // pi * 2
const double pi_div_by_2 = 1.5707963267948966;      // pi / 2
const double pi_div_by_4 = 0.7853981633974483;      // pi / 4



//Air packet structure and fields position
#define PACKET_NUM_ID_POS           (0)			//byte pos in a packet
#define BYTE_NUM_POS           		(5)			//bits pos in a byte
#define PACKET_NUM_MASK           	(0xE0)
#define PACKET_ID_MASK           	(0x1F)
#define PACKET_FLAGS_POS            (1)
#define PACKET_LATITUDE_POS         (2)
#define PACKET_LONGITUDE_POS        (6)
#define PACKET_ALTITUDE_POS        	(10)
//1 byte (0) device number and ID (single char)
//1 byte (1) flags
//4 bytes (2, 3, 4, 5) lat
//4 bytes (6, 7, 8, 9) lon
//2 bytes (10, 11) altitude
//TOTAL 12 bytes - see radio.c for AIR_PACKET_LEN



struct settings_struct *p_settings;
uint8_t *p_air_packet_tx;
uint8_t *p_air_packet_rx;

uint8_t this_device;
struct devices_struct devices[NAV_OBJECTS_MAX + 1];        //structures array for devices from 1 to DEVICES_IN_GROUP. Index 0 is invalid and always empty.
struct devices_struct *p_devices[NAV_OBJECTS_MAX + 1];		//structure pointers array



void init_lrns(void)
{
	//Clear mem
    for (uint8_t dev = 1; dev <= NAV_OBJECTS_MAX; dev++)
    {
        memset(&devices[dev], 0, sizeof(devices[dev]));
        devices[dev].rx_icon = SYMB16_RX13;
    }

	//Get external things
	p_settings = get_settings();
	p_air_packet_tx = get_air_packet_tx();
	p_air_packet_rx = get_air_packet_rx();


	//This device number
	this_device = p_settings->device_number;

    //Activate this device
	devices[this_device].exist_flag = 1;
	devices[this_device].device_id = p_settings->device_id;
}







void fill_air_packet(void)
{
	p_air_packet_tx[PACKET_NUM_ID_POS] = 			(this_device << BYTE_NUM_POS) | (devices[this_device].device_id - 'A');	   //transmit dev id as A-Z, but with 0x41 ('A') shift resulting in 0-25 dec

	p_air_packet_tx[PACKET_FLAGS_POS] = 			0; 			//todo add flags here

	p_air_packet_tx[PACKET_LATITUDE_POS] = 			devices[this_device].latitude.as_array[0];
	p_air_packet_tx[PACKET_LATITUDE_POS + 1] = 		devices[this_device].latitude.as_array[1];
	p_air_packet_tx[PACKET_LATITUDE_POS + 2] = 		devices[this_device].latitude.as_array[2];
	p_air_packet_tx[PACKET_LATITUDE_POS + 3] = 		devices[this_device].latitude.as_array[3];

	p_air_packet_tx[PACKET_LONGITUDE_POS] = 		devices[this_device].longitude.as_array[0];
	p_air_packet_tx[PACKET_LONGITUDE_POS + 1] = 	devices[this_device].longitude.as_array[1];
	p_air_packet_tx[PACKET_LONGITUDE_POS + 2] = 	devices[this_device].longitude.as_array[2];
	p_air_packet_tx[PACKET_LONGITUDE_POS + 3] = 	devices[this_device].longitude.as_array[3];

	p_air_packet_tx[PACKET_ALTITUDE_POS] = 			devices[this_device].altitude.as_array[0];
	p_air_packet_tx[PACKET_ALTITUDE_POS + 1] = 		devices[this_device].altitude.as_array[1];
}



void parse_air_packet(void)
{
	uint8_t rx_device = (p_air_packet_rx[PACKET_NUM_ID_POS] & PACKET_NUM_MASK) >> BYTE_NUM_POS; //extract device number from received packet

	devices[rx_device].exist_flag 				=	1;
	devices[rx_device].device_id				=	(p_air_packet_rx[PACKET_NUM_ID_POS] & PACKET_ID_MASK) + 'A';	//restore 0x41 shift

	//todo read flags here

	devices[rx_device].latitude.as_array[0]	=		p_air_packet_rx[PACKET_LATITUDE_POS];
	devices[rx_device].latitude.as_array[1]	=		p_air_packet_rx[PACKET_LATITUDE_POS + 1];
	devices[rx_device].latitude.as_array[2]	=		p_air_packet_rx[PACKET_LATITUDE_POS + 2];
	devices[rx_device].latitude.as_array[3]	=		p_air_packet_rx[PACKET_LATITUDE_POS + 3];

	devices[rx_device].longitude.as_array[0]	=	p_air_packet_rx[PACKET_LONGITUDE_POS];
	devices[rx_device].longitude.as_array[1]	=	p_air_packet_rx[PACKET_LONGITUDE_POS + 1];
	devices[rx_device].longitude.as_array[2]	=	p_air_packet_rx[PACKET_LONGITUDE_POS + 2];
	devices[rx_device].longitude.as_array[3]	=	p_air_packet_rx[PACKET_LONGITUDE_POS + 3];

	devices[rx_device].altitude.as_array[0] =		p_air_packet_rx[PACKET_ALTITUDE_POS];
	devices[rx_device].altitude.as_array[1] = 		p_air_packet_rx[PACKET_ALTITUDE_POS + 1];


	update_reception_icon(rx_device);
}



void update_reception_icon(uint8_t rx_device)
{
	if (devices[rx_device].rx_icon == SYMB16_RX33)
	{
		devices[rx_device].rx_icon = SYMB16_RX13;
	}
	else
	{
		devices[rx_device].rx_icon++;
	}
}



void process_all_devices(void)
{
	for (uint8_t dev = 1; dev <= NAV_OBJECTS_MAX; dev++)
	{
		if (dev == this_device)	//except this device
		{
			continue;
		}

		if (devices[dev].exist_flag == 1)	//all existing
		{
			calc_relative_position(dev);
		}
	}
}



void process_current_device(void)
{

	uint8_t curr_dev = get_current_device();

	if (curr_dev != this_device)
	{
		calc_relative_position(curr_dev);
	}

}



//  Heart of the device <3
//Calculates distance between two GPS points (using haversine formula)
//Calculates heading between two GPS points (loxodrome, or rhumb line)
//Calculates altitude difference
void calc_relative_position(uint8_t another_device)
{
    double lat1, lat2, lon1, lon2;
    double distance, heading_deg, heading_rad;

    //my position
    lat1 = devices[this_device].latitude.as_float * deg_to_rad;
    lon1 = devices[this_device].longitude.as_float * deg_to_rad;

    //position of the device to calculate relative position
    lat2 = devices[another_device].latitude.as_float * deg_to_rad;
    lon2 = devices[another_device].longitude.as_float * deg_to_rad;

    if (lat1 == lat2)
    {
        lat2 += 0.00000001;       //slightly shift the position
    }

    if (lon1 == lon2)
    {
        lon2 += 0.00000001;       //slightly shift the position
    }

    distance = twice_mean_earth_radius *
               asin( sqrt( pow(sin((lat2 - lat1) / 2), 2) +
                           cos(lat2) * cos(lat1) * pow(sin((lon2 - lon1) / 2), 2)));

    heading_rad = atan((lon2 - lon1) /
                    log(tan(pi_div_by_4 + lat2 / 2) / tan(pi_div_by_4 + lat1 / 2)));



    if ((lat2 > lat1) && (lon2 > lon1))
    {
        ;
    }
    else if ((lat2 > lat1) && (lon2 < lon1))
    {
    	heading_rad += pi_mul_by_2;
    }
    else if (lat2 < lat1)
    {
    	heading_rad += pi_div_by_1;
    }



    heading_deg = heading_rad * rad_to_deg;		//convert to deg

    devices[another_device].distance = (uint32_t)distance;

    if (devices[another_device].distance == 0)
    {
    	devices[another_device].heading_deg = 0;	//if distance is zero then make heading equals zero too
    	devices[another_device].heading_rad = 0;
    }
    else
    {
    	devices[another_device].heading_deg = (uint16_t)heading_deg;
    	devices[another_device].heading_rad = (float)heading_rad;
    }



    //altitude diff
    devices[another_device].delta_altitude = devices[another_device].altitude.as_integer - devices[this_device].altitude.as_integer;

}



void toggle_alarm(void)
{
	if (devices[this_device].alarm_flag == 0)
	{
		devices[this_device].alarm_flag = 1;
	}
	else
	{
		devices[this_device].alarm_flag = 0;
	}
}



uint8_t get_alarm_status(void)
{
	return devices[this_device].alarm_flag;
}



struct devices_struct **get_devices(void)
{
	for (uint8_t dev = 0; dev <= NAV_OBJECTS_MAX; dev++)
	{
		p_devices[dev] = &devices[dev];
	}

	return &p_devices[0];
}
