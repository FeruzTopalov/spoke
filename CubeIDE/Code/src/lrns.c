/*
    SPOKE

    file: lrns.c
*/



#include <string.h>
#include <math.h>
#include "stm32f10x.h"
#include "lrns.h"
#include "settings.h"
#include "gps.h"
#include "radio.h"
#include "menu.h"
#include "lcd.h"
#include "gpio.h"
#include "main.h"



const double rad_to_deg = 57.29577951308232;        //rad to deg multiplyer
const double deg_to_rad = 0.0174532925199433;       //deg to rad multiplyer
const double twice_mean_earth_radius = 12742016.0;  // 2 * 6371008 meters
const double pi_div_by_1 = 3.141592653589793;      // pi / 1
const double pi_mul_by_2 = 6.283185307179586;      // pi * 2
const double pi_div_by_2 = 1.5707963267948966;      // pi / 2
const double pi_div_by_4 = 0.7853981633974483;      // pi / 4



//Air packet structure and fields position
//1 byte (0) header =  and ID (single char)
//pos 0, size 1 - flags
//pos 1, size 1 - device number
//pos 2, size 1 - device ID
//pos 3-6, size 4 - latitude
//pos 7-10, size 4 - longitude
//pos 11-12, size 2 - altitude
//TOTAL 13 bytes - see radio.c for AIR_PACKET_LEN

//Bytes positions in a packet
#define PKT_HEADER_0_POS           (0)
#define PKT_HEADER_1_POS           (1)
#define PKT_HEADER_2_POS           (2)
#define PKT_LATITUDE_0_POS         (3)
#define PKT_LATITUDE_1_POS         (4)
#define PKT_LATITUDE_2_POS         (5)
#define PKT_LATITUDE_3_POS         (6)
#define PKT_LONGITUDE_0_POS        (7)
#define PKT_LONGITUDE_1_POS        (8)
#define PKT_LONGITUDE_2_POS        (9)
#define PKT_LONGITUDE_3_POS        (10)
#define PKT_ALTITUDE_0_POS         (11)
#define PKT_ALTITUDE_1_POS         (12)

//Bits positions in bytes
#define BYT_HEADER_0_FLAG_ALARM_POS  		(0)
#define BYT_HEADER_0_FLAG_ALARM_MASK		(0b00000001)

#define BYT_HEADER_0_FLAG_BEACON_POS  		(1)
#define BYT_HEADER_0_FLAG_BEACON_MASK		(0b00000010)

#define BYT_HEADER_0_FLAG_LOW_BAT_POS  		(2)
#define BYT_HEADER_0_FLAG_LOW_BAT_MASK		(0b00000100)

#define BYT_HEADER_0_FLAG_ACC_MOVE_POS  	(3)
#define BYT_HEADER_0_FLAG_ACC_MOVE_MASK		(0b00001000)

#define BYT_HEADER_0_FLAG_PDOP_POS  		(4)
#define BYT_HEADER_0_FLAG_PDOP_MASK			(0b00010000)

#define BYT_HEADER_0_RES_RES_POS  			(5)
#define BYT_HEADER_0_RES_RES_MASK			(0b11100000)

#define BYT_HEADER_1_DEV_NUM_POS  			(0)
#define BYT_HEADER_1_DEV_NUM_MASK			(0b00011111)

#define BYT_HEADER_1_RES_RES_POS  			(5)
#define BYT_HEADER_1_RES_RES_MASK			(0b11100000)

#define BYT_HEADER_2_DEV_ID_POS  			(0)
#define BYT_HEADER_2_DEV_ID_MASK			(0b00011111)

#define BYT_HEADER_2_RES_RES_POS  			(5)
#define BYT_HEADER_2_RES_RES_MASK			(0b11100000)



struct settings_struct *p_settings;
uint8_t *p_air_packet_tx;
uint8_t *p_air_packet_rx;

uint8_t this_device;
struct devices_struct devices[NAV_OBJECTS_MAX + 1];        //structures array for devices from 1 to DEVICES_IN_GROUP. Index 0 is invalid and always empty.
struct devices_struct *p_devices[NAV_OBJECTS_MAX + 1];		//structure pointers array

uint8_t *p_update_interval_values;



void init_lrns(void)
{
	//Clear mem
    for (uint8_t dev = 1; dev <= NAV_OBJECTS_MAX; dev++)
    {
        memset(&devices[dev], 0, sizeof(devices[dev]));
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
	devices[this_device].beacon_flag = 0; //device is not a beacon

	//Get update interval
	p_update_interval_values = get_update_interval_values();
}



void fill_air_packet(uint32_t current_uptime)
{
	devices[this_device].timestamp = current_uptime;

	p_air_packet_tx[PKT_HEADER_0_POS] = 			(devices[this_device].alarm_flag << BYT_HEADER_0_FLAG_ALARM_POS) |
													(devices[this_device].beacon_flag << BYT_HEADER_0_FLAG_BEACON_POS) |
													(devices[this_device].lowbat_flag << BYT_HEADER_0_FLAG_LOW_BAT_POS) |
													(devices[this_device].acc_movement_flag << BYT_HEADER_0_FLAG_ACC_MOVE_POS) |
													(devices[this_device].pdop_flag << BYT_HEADER_0_FLAG_PDOP_POS); //todo set pdop flag in gps parse()									;

	p_air_packet_tx[PKT_HEADER_1_POS] = 			(this_device << BYT_HEADER_1_DEV_NUM_POS);

	//transmit dev id as A-Z, but with 0x41 ('A') shift resulting in 0-25 dec
	p_air_packet_tx[PKT_HEADER_2_POS] = 			((devices[this_device].device_id - 'A') << BYT_HEADER_2_DEV_ID_POS);

	p_air_packet_tx[PKT_LATITUDE_0_POS] = 			devices[this_device].latitude.as_array[0];
	p_air_packet_tx[PKT_LATITUDE_1_POS] = 			devices[this_device].latitude.as_array[1];
	p_air_packet_tx[PKT_LATITUDE_2_POS] = 			devices[this_device].latitude.as_array[2];
	p_air_packet_tx[PKT_LATITUDE_3_POS] = 			devices[this_device].latitude.as_array[3];

	p_air_packet_tx[PKT_LONGITUDE_0_POS] = 			devices[this_device].longitude.as_array[0];
	p_air_packet_tx[PKT_LONGITUDE_1_POS] = 			devices[this_device].longitude.as_array[1];
	p_air_packet_tx[PKT_LONGITUDE_2_POS] = 			devices[this_device].longitude.as_array[2];
	p_air_packet_tx[PKT_LONGITUDE_3_POS] = 			devices[this_device].longitude.as_array[3];

	p_air_packet_tx[PKT_ALTITUDE_0_POS] = 			devices[this_device].altitude.as_array[0];
	p_air_packet_tx[PKT_ALTITUDE_1_POS] = 			devices[this_device].altitude.as_array[1];

	set_acc_movement_flag(ACC_MOVEMENT_NOT_DETECTED);	//clear flag only after the acc movement flag was written to tx buf
	enable_acc_movement_interrupt();					//enable acc interrupt
}



uint8_t parse_air_packet(uint32_t current_uptime)
{
	uint8_t temp_alarm_flag = 0;
	uint8_t rx_device = (p_air_packet_rx[INPACKET_HEADER_POS] & INBYTE_HEADER_NUM_MASK) >> INBYTE_HEADER_NUM_POS; //extract device number from received packet

	devices[rx_device].exist_flag 				=	1;
	devices[rx_device].device_id				=	((p_air_packet_rx[INPACKET_HEADER_POS] & INBYTE_HEADER_ID_MASK) + 'A') >> INBYTE_HEADER_ID_POS;	//restore 0x41 shift
	devices[rx_device].timestamp				=	current_uptime;

	temp_alarm_flag 							=	(p_air_packet_rx[INPACKET_FLAGS_POS] & INBYTE_FLAGS_ALARM_MASK) >> INBYTE_FLAGS_ALARM_POS;

	if (temp_alarm_flag == 1)
	{
		if (devices[rx_device].alarm_flag == 0)
		{
			devices[rx_device].alarm_flag_for_beep = 1;
		}
	}
	else
	{
		devices[rx_device].alarm_flag_for_beep = 0;
	}

	devices[rx_device].alarm_flag 				=	temp_alarm_flag;
	devices[rx_device].lowbat_flag				=	(p_air_packet_rx[INPACKET_FLAGS_POS] & INBYTE_FLAGS_LOWBAT_MASK) >> INBYTE_FLAGS_LOWBAT_POS;

	devices[rx_device].latitude.as_array[0]	=		p_air_packet_rx[INPACKET_LATITUDE_POS];
	devices[rx_device].latitude.as_array[1]	=		p_air_packet_rx[INPACKET_LATITUDE_POS + 1];
	devices[rx_device].latitude.as_array[2]	=		p_air_packet_rx[INPACKET_LATITUDE_POS + 2];
	devices[rx_device].latitude.as_array[3]	=		p_air_packet_rx[INPACKET_LATITUDE_POS + 3];

	devices[rx_device].longitude.as_array[0]	=	p_air_packet_rx[INPACKET_LONGITUDE_POS];
	devices[rx_device].longitude.as_array[1]	=	p_air_packet_rx[INPACKET_LONGITUDE_POS + 1];
	devices[rx_device].longitude.as_array[2]	=	p_air_packet_rx[INPACKET_LONGITUDE_POS + 2];
	devices[rx_device].longitude.as_array[3]	=	p_air_packet_rx[INPACKET_LONGITUDE_POS + 3];

	devices[rx_device].altitude.as_array[0] =		p_air_packet_rx[INPACKET_ALTITUDE_POS];
	devices[rx_device].altitude.as_array[1] = 		p_air_packet_rx[INPACKET_ALTITUDE_POS + 1];

	return rx_device;
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



void calc_timeout(uint32_t current_uptime)
{
	uint8_t existing_devs = 0;		//to count active devs

	for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)	//calculated even for this device and used to alarm about own timeout upon lost of PPS signal
	{
		if (devices[dev].exist_flag == 1)
		{
			existing_devs++;

			devices[dev].timeout = current_uptime - devices[dev].timestamp; //calc timeout for each active device

			//check link status
			if (devices[dev].timeout > p_update_interval_values[p_settings->update_interval_opt])
			{
				devices[dev].link_status_flag = 0; //when rx from device did not happen but timeout has not triggered yet we show questionmark
			}
			else
			{
				devices[dev].link_status_flag = 1; //show checkmark if the device is "online"
			}

			//assign timeout flag
        	if (p_settings->timeout_threshold != TIMEOUT_ALARM_DISABLED) //if enabled
        	{
				if (devices[dev].timeout > (p_settings->timeout_threshold + 1))		//safety +1 for boundary condition alarm prevention
				{
					if (devices[dev].timeout_flag == 0)
					{
						devices[dev].timeout_flag = 1; //set flag for alarm
						devices[dev].timeout_flag_for_beep = 1;
					}
				}
				else
				{
					devices[dev].timeout_flag = 0;
					devices[dev].timeout_flag_for_beep = 0;
				}
        	}
        }
    }

	if (existing_devs == 1)	//if only our dev exists
	{
		devices[this_device].timeout_flag = 0;				//clear our timeout flags to not to beep for timeout when there are no one who would be informed about our desappear
		devices[this_device].timeout_flag_for_beep = 0;
	}
}



void calc_fence(void)		//all devices should be processed before calling this func
{
	if (p_settings->fence_threshold != FENCE_ALARM_DISABLED)
	{
		for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)		//devices only, not for mem point
		{
			if (devices[dev].exist_flag)
			{
				if (devices[dev].distance > p_settings->fence_threshold)
				{
					if (devices[dev].fence_flag == 0)
					{
						devices[dev].fence_flag = 1;
						devices[dev].fence_flag_for_beep = 1;
					}
				}
				else
				{
					devices[dev].fence_flag = 0;
					devices[dev].fence_flag_for_beep = 0;
				}
			}
		}
	}
}



uint8_t check_any_alarm_fence_timeout(void)
{
	for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)
	{
		if (devices[dev].exist_flag)
		{
			if ((devices[dev].alarm_flag_for_beep) || (devices[dev].fence_flag_for_beep) || (devices[dev].timeout_flag_for_beep))
			{
				return 1;
			}
		}
	}

	return 0;
}



void clear_beep_for_active_dev(uint8_t active_device)
{
    //clear beep for a device shown on the screen
    devices[active_device].alarm_flag_for_beep = 0;
    devices[active_device].fence_flag_for_beep = 0;
    devices[active_device].timeout_flag_for_beep = 0;
}



void toggle_my_alarm(void)
{
	if (devices[this_device].alarm_flag == 0)
	{
		devices[this_device].alarm_flag = 1;
		devices[this_device].alarm_flag_for_beep = 1;
	}
	else
	{
		devices[this_device].alarm_flag = 0;
		devices[this_device].alarm_flag_for_beep = 0;
	}
}



void enable_my_alarm(void)
{
	devices[this_device].alarm_flag = 1;
	devices[this_device].alarm_flag_for_beep = 1;
}



uint8_t get_my_alarm_status(void)
{
	return devices[this_device].alarm_flag;
}



void set_lowbat_flag(uint8_t value)
{
	devices[this_device].lowbat_flag = value;
}



void set_acc_movement_flag(uint8_t value)
{
	devices[this_device].acc_movement_flag = value;
}



struct devices_struct **get_devices(void)
{
	for (uint8_t dev = 0; dev <= NAV_OBJECTS_MAX; dev++)
	{
		p_devices[dev] = &devices[dev];
	}

	return &p_devices[0];
}
