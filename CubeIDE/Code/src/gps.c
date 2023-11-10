/*
    SPOKE
    
    file: gps.c
*/



#include <string.h>
#include "stm32f10x.h"
#include "main.h"
#include "gps.h"
#include "uart.h"
#include "service.h"
#include "settings.h"
#include "points.h"
#include "lrns.h"
#include "calendar.h"
#include "ubx.h"



void gps_raw_convert_to_numerical(void);
void configure_gps_receiver(void);
void send_ubx(uint8_t class, uint8_t id, uint8_t payload[], uint8_t len);
void update_my_coordinates(void);
void convert_time(void);



const float knots_to_kph = 1.852;                   //knots to kilometers per hour multiplyer



uint8_t nmea_checksum(uint16_t pos);
uint8_t parse_RMC(void);
uint8_t parse_GGA(void);
uint8_t parse_GSA(void);
uint8_t parse_GSV(void);



char nmea_data[UART_BUF_LEN];
struct gps_raw_struct gps_raw;
struct gps_num_struct gps_num;
struct settings_struct *p_settings;
struct devices_struct **pp_devices;


uint8_t this_device;
char tmp_char[15];
uint8_t tmp_uint8;
float tmp_float;


//GPS CONFIGURATION
//GPS module-specific, edit according your module and mode of operation you need
//works for u-blox m8 chip with protocol version 18, particularly for GN-801 GPS module
//generated via u-center ver 23.08

//This resets config to default (clear operation)
uint8_t cfg_cfg_config_reset[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};

//This saves config to BBR and Flash (save operation)
uint8_t cfg_cfg_config_save[] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};

//This performs forced HW restart + coldstart
uint8_t cfg_rst_cold_restart[] = {0xFF, 0xB9, 0x00, 0x00};

//Set PPS timepulse length locked to 999900 us, so the module's LED will blink for only 100 us
//This gives loong positive pulse, and short negative pulse where the LED is on
uint8_t cfg_tp5_timepulse[] = {0x00, 0x01, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDC, 0x41, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00};

//This enables Aggressive 1 Hz power saving mode
uint8_t cfg_pms_aggr1hz[] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//This disables SBAS and GLONASS (i.e. GPS + QZSS mode), To avoid cross-correlation issues, it is recommended that GPS and QZSS are always both enabled or both disabled
uint8_t cfg_gnss_gps[] = {0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01};

//This disables SBAS (i.e. GPS + QZSS + GLONASS mode), To avoid cross-correlation issues, it is recommended that GPS and QZSS are always both enabled or both disabled
uint8_t cfg_gnss_gps_glonass[] = {0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x01, 0x00, 0x01, 0x01};



void gps_init(void)
{
	//Load all devices
	pp_devices = get_devices();

	p_settings = get_settings();
	this_device = p_settings->device_number;		//todo overall make this device number get as a function or as a more beautiful way

	configure_gps_receiver();
}



void configure_gps_receiver(void)
{
	;
}



void reset_to_defaults_gps_receiver(void)
{
	//GPS module-specific, edit according your module and mode of operation you need

	send_ubx(UBX_CLASS_CFG, UBX_CFG_CFG, &cfg_cfg_config_reset[0], sizeof(cfg_cfg_config_reset));
	delay_cyc(100000);

	send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
	delay_cyc(100000);
}



void send_ubx(uint8_t class, uint8_t id, uint8_t payload[], uint8_t len)
{
	uint8_t ubx_message[255 + 8] = {0};
	uint8_t CK_A = 0;
	uint8_t CK_B = 0;

	ubx_message[0] = UBX_SYNCH_1;
	ubx_message[1] = UBX_SYNCH_2;
	ubx_message[2] = class;
	ubx_message[3] = id;
	ubx_message[4] = len;		//length of payload field LSB (expected len < 256)
	ubx_message[5] = 0;			//length of payload field MSB
	ubx_message[6] = 0;			//payload starts from here

	for (uint8_t i = 0; i < len; i++)
	{
		ubx_message[6 + i] = payload[i];	//copy payload
	}

	for (uint8_t m = 2; m < (len + 6); m++)
	{
		CK_A = CK_A + ubx_message[m];		//calc checksums
		CK_B = CK_B + CK_A;
	}

	ubx_message[6 + len] = CK_A;
	ubx_message[6 + len + 1] = CK_B;

	for (uint8_t n = 0; n < (len + 8); n++)		//8 bytes header & checksum
	{
		uart3_tx_byte(ubx_message[n]);			//transmit
		uart1_tx_byte(ubx_message[n]);//debug
	}
}



//Parse all gps fields all together
uint8_t parse_gps(void)
{
	memset(&gps_raw, 0, sizeof(gps_raw));
	if (parse_RMC() && parse_GGA() && parse_GSA() && parse_GSV())
	{
		memset(&gps_num, 0, sizeof(gps_num));
		gps_raw_convert_to_numerical();
		update_my_coordinates();
		return 1;
	}
	else
	{
		return 0;
	}
}



//Converts gps_raw data (symbols) to gps_num data (numbers) with conversions if needed
void gps_raw_convert_to_numerical(void)
{
    //Time
    tmp_char[0] = gps_raw.time[0];
    tmp_char[1] = gps_raw.time[1];
    tmp_char[2] = 0;
    gps_num.hour = (uint8_t)atoi32(&tmp_char[0]);

    tmp_char[0] = gps_raw.time[2];
    tmp_char[1] = gps_raw.time[3];
    tmp_char[2] = 0;
    gps_num.minute = (uint8_t)atoi32(&tmp_char[0]);

    tmp_char[0] = gps_raw.time[4];
    tmp_char[1] = gps_raw.time[5];
    tmp_char[2] = 0;
    gps_num.second = (uint8_t)atoi32(&tmp_char[0]);

    //Date
    tmp_char[0] = gps_raw.date[0];
    tmp_char[1] = gps_raw.date[1];
    tmp_char[2] = 0;
    gps_num.day = (uint8_t)atoi32(&tmp_char[0]);

    tmp_char[0] = gps_raw.date[2];
    tmp_char[1] = gps_raw.date[3];
    tmp_char[2] = 0;
    gps_num.month = (uint8_t)atoi32(&tmp_char[0]);

    tmp_char[0] = gps_raw.date[4];
    tmp_char[1] = gps_raw.date[5];
    tmp_char[2] = 0;
    gps_num.year = (uint8_t)atoi32(&tmp_char[0]);

    convert_time();	//convert UTC time to local

    //Latitude
    tmp_char[0] = gps_raw.latitude[0];
    tmp_char[1] = gps_raw.latitude[1];
    tmp_char[2] = 0;
    tmp_uint8 = (uint8_t)atoi32(&tmp_char[0]);          //int part of lat
    tmp_float = atof32(&(gps_raw.latitude[2]));        //frac part of lat

    tmp_float /= 60.0;                                  //convert ddmm.mmmm to dd.dddddd
    tmp_float += tmp_uint8;

    if (gps_raw.ns[0] == 'S')
    {
        gps_num.latitude = tmp_float * -1.0;    //negative (southern) latitudes
    }
    else
    {
        gps_num.latitude = tmp_float;
    }

    //Longitude
    tmp_char[0] = gps_raw.longitude[0];
    tmp_char[1] = gps_raw.longitude[1];
    tmp_char[2] = gps_raw.longitude[2];
    tmp_char[3] = 0;
    tmp_uint8 = (uint8_t)atoi32(&tmp_char[0]);          //int part of lon
    tmp_float = atof32(&(gps_raw.longitude[3]));       //frac part of lon

    tmp_float /= 60.0;                                  //convert dddmm.mmmm to ddd.dddddd
    tmp_float += tmp_uint8;

    if (gps_raw.ew[0] == 'W')
    {
        gps_num.longitude = tmp_float * -1.0;   //negative (western) longitudes
    }
    else
    {
        gps_num.longitude = tmp_float;
    }

    //Speed
    gps_num.speed = atof32(&(gps_raw.speed[0])) * knots_to_kph;

    //Course
    gps_num.course = atof32(&(gps_raw.course[0]));

    //Altitude
    gps_num.altitude = atof32(&(gps_raw.altitude[0]));

    //Satellites
    gps_num.sat_view = (uint8_t)atoi32(&(gps_raw.sat_view[0]));
    gps_num.sat_used = (uint8_t)atoi32(&(gps_raw.sat_used[0]));

    //Status, Mode, PDOP
    if (gps_raw.status[0] == 'A')
    {
        gps_num.status = GPS_DATA_VALID;
    }
    else
    {
        gps_num.status = GPS_DATA_INVALID;
    }

    gps_num.mode = (uint8_t)atoi32(&(gps_raw.mode[0]));

    gps_num.pdop = atof32(&(gps_raw.pdop[0]));
}



void convert_time(void)
{
	int8_t minute = gps_num.minute;
	int8_t hour = gps_num.hour;
	int8_t day = gps_num.day;
	int8_t month = gps_num.month;
	int8_t year = gps_num.year;

	if (day == 0)	//invalid data
	{
		return;
	}

	if (p_settings->time_zone_dir != 0)
	{
		if (p_settings->time_zone_dir > 0)
		{
			minute += p_settings->time_zone_minute;

			if (minute > 59)
			{
				minute -= 60;
				hour += 1;
			}

			hour += p_settings->time_zone_hour;

			if (hour > 23)
			{
				hour -= 24;
				day += 1;

				if (day > calendar[year - CALENDAR_START_YEAR][month - 1])
				{
					day = 1;
					month += 1;

					if (month > 12)
					{
						month = 1;
						year += 1;
					}
				}
			}
		}
		else
		{
			minute -= p_settings->time_zone_minute;

			if (minute < 0)
			{
				minute += 60;
				hour -= 1;
			}

			hour -= p_settings->time_zone_hour;

			if (hour < 0)
			{
				hour += 24;
				day -= 1;

				if (day < 1)
				{
					month -= 1;

					if (month < 1)
					{
						month = 12;
						year -= 1;
					}

					day = calendar[year - CALENDAR_START_YEAR][month - 1];
				}
			}
		}
	}

	gps_num.minute_tz = minute;
	gps_num.hour_tz = hour;

	gps_num.day_tz = day;
	gps_num.month_tz = month;
	gps_num.year_tz = year;

}






//Update devices structure with coordinates of this device in order to transmit them further
void update_my_coordinates(void)
{
    //Latitude & Longitude
	pp_devices[this_device]->latitude.as_float = gps_num.latitude;
	pp_devices[this_device]->longitude.as_float = gps_num.longitude;

	//Altitude
	pp_devices[this_device]->altitude.as_integer = (int16_t)gps_num.altitude;
}



//Parse RMC sentence
uint8_t parse_RMC(void)
{
    uint8_t comma = 0;
    uint8_t sym = 0;
    uint16_t pos = 0;
    
    while (!((nmea_data[pos] == '$') &&
            (nmea_data[pos + 3] == 'R') &&
            (nmea_data[pos + 4] == 'M') &&
            (nmea_data[pos + 5] == 'C')) && pos < UART_BUF_LEN)      //search for start pos
    {
    pos++;
    }
    
    if(nmea_checksum(pos) == 0)
    {
        return 0;           //checksum error
    }
    
    for (uint16_t i = pos + 6; i < UART_BUF_LEN ; i++)          //i starts from the symbol right after "$GPRMC" string
    {
        if (nmea_data[i] == '*') return 1;       //end of the sentence
        
        if (nmea_data[i] == ',')
        {
            comma++;        //increase comma counter
            sym = 0;
        }
        else
        {
            switch (comma)  //surfing through RMC data fields
            {
                case 1:
                    gps_raw.time[sym++] = nmea_data[i];
                    break;
                case 2:
                    gps_raw.status[sym++] = nmea_data[i];
                    break;
                case 3:
                    gps_raw.latitude[sym++] = nmea_data[i];
                    break;
                case 4:
                    gps_raw.ns[sym++] = nmea_data[i];
                    break;
                case 5:
                    gps_raw.longitude[sym++] = nmea_data[i];
                    break;
                case 6:
                    gps_raw.ew[sym++] = nmea_data[i];
                    break;
                case 7:
                    gps_raw.speed[sym++] = nmea_data[i];
                    break;
                case 8:
                    gps_raw.course[sym++] = nmea_data[i];
                    break;
                case 9:
                    gps_raw.date[sym++] = nmea_data[i];
                    break;
                default:
                    break;
            }
        }
    }
    return 1;
}



//Parse GGA sentence
uint8_t parse_GGA(void)
{
    uint8_t comma = 0;
    uint8_t sym = 0;
    uint16_t pos = 0;
    
    while (!((nmea_data[pos] == '$') &&
            (nmea_data[pos + 3] == 'G') &&
            (nmea_data[pos + 4] == 'G') &&
            (nmea_data[pos + 5] == 'A')) && pos < UART_BUF_LEN)      //search for start pos
    {
    pos++;
    }
    
    if(nmea_checksum(pos) == 0)
    {
        return 0;           //checksum error
    }
    
    for (uint16_t i = pos + 6; i < UART_BUF_LEN ; i++)          //i starts from the symbol right after "$GPRMC" string
    {
        if (nmea_data[i] == '*') return 1;       //end of the sentence
        
        if (nmea_data[i] == ',')
        {
            comma++;        //increase comma counter
            sym = 0;
        }
        else
        {
            switch (comma)  //surfing through GGA data fields
            {
                case 7:
                    gps_raw.sat_used[sym++] = nmea_data[i];
                    break;
                case 9:
                    gps_raw.altitude[sym++] = nmea_data[i];
                    break;
                default:
                    break;
            }
        }
    }
    return 1;
}



//Parse GSA sentence
uint8_t parse_GSA(void)
{
    uint8_t comma = 0;
    uint8_t sym = 0;
    uint16_t pos = 0;
    
    while (!((nmea_data[pos] == '$') &&
            (nmea_data[pos + 3] == 'G') &&
            (nmea_data[pos + 4] == 'S') &&
            (nmea_data[pos + 5] == 'A')) && pos < UART_BUF_LEN)      //search for start pos
    {
    pos++;
    }
    
    if(nmea_checksum(pos) == 0)
    {
        return 0;           //checksum error
    }
    
    for (uint16_t i = pos + 6; i < UART_BUF_LEN ; i++)          //i starts from the symbol right after "$GPRMC" string
    {
        if (nmea_data[i] == '*') return 1;       //end of the sentence
        
        if (nmea_data[i] == ',')
        {
            comma++;        //increase comma counter
            sym = 0;
        }
        else
        {
            switch (comma)  //surfing through GSA data fields
            {
                case 2:
                    gps_raw.mode[sym++] = nmea_data[i];
                    break;
                case 15:
                    gps_raw.pdop[sym++] = nmea_data[i];
                    break;
                default:
                    break;
            }
        }
    }
    return 1;
}



//Parse GSV sentence
uint8_t parse_GSV(void)
{
    uint8_t comma = 0;
    uint8_t sym = 0;
    uint16_t pos = 0;
    
    while (!((nmea_data[pos] == '$') &&
            (nmea_data[pos + 3] == 'G') &&
            (nmea_data[pos + 4] == 'S') &&
            (nmea_data[pos + 5] == 'V')) && pos < UART_BUF_LEN)      //search for start pos
    {
    pos++;
    }
    
    if(nmea_checksum(pos) == 0)
    {
        return 0;           //checksum error
    }
    
    for (uint16_t i = pos + 6; i < UART_BUF_LEN ; i++)          //i starts from the symbol right after "$GPRMC" string
    {
        if (nmea_data[i] == '*') return 1;       //end of the sentence
        
        if (nmea_data[i] == ',')
        {
            comma++;        //increase comma counter
            sym = 0;
        }
        else
        {
            switch (comma)  //surfing through GSV data fields
            {
                case 3:
                    gps_raw.sat_view[sym++] = nmea_data[i];
                    break;
                default:
                    break;
            }
        }
    }
    return 1;
}



//NMEA checksum
uint8_t nmea_checksum(uint16_t pos)
{
    uint8_t CheckSum = 0;
    uint8_t MessageCheckSum = 0;
    
    pos++;                                      //pick up symbol right after $
    while (nmea_data[pos] != '*')
    {
        CheckSum ^= nmea_data[pos];
        if (++pos >= UART_BUF_LEN) return 0;    //check sum not found
    }
    
    if (nmea_data[++pos] > 0x40)
    {
        MessageCheckSum = (nmea_data[pos] - 0x37) << 4;   //ascii hex to number
    }
    else
    {
        MessageCheckSum = (nmea_data[pos] - 0x30) << 4;
    }
    
    if (nmea_data[++pos] > 0x40)
    {
        MessageCheckSum += (nmea_data[pos] - 0x37);       //ascii hex to number
    }
    else
    {
        MessageCheckSum += (nmea_data[pos] - 0x30);
    }
    
    if (MessageCheckSum == CheckSum)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



char *get_nmea_buf(void)
{
	return &nmea_data[0];
}



struct gps_raw_struct *get_gps_raw(void)
{
	return &gps_raw;
}



struct gps_num_struct *get_gps_num(void)
{
	return &gps_num;
}
