/*
    SPOKE

    file: settings.h
*/



#ifndef SETTINGS_HEADER
#define SETTINGS_HEADER



#define TIMEOUT_ALARM_DISABLED		(0)
#define FENCE_ALARM_DISABLED		(0)



//Update interval settings
#define UPDATE_INTERVAL_10S_SETTING		(0)
#define UPDATE_INTERVAL_30S_SETTING		(1)
#define UPDATE_INTERVAL_60S_SETTING		(2)

#define UPDATE_INTERVAL_FIRST_OPTION 	(UPDATE_INTERVAL_10S_SETTING)
#define UPDATE_INTERVAL_LAST_OPTION 	(UPDATE_INTERVAL_60S_SETTING)


//POWER SETTINGS
#define TX_POWER_NEG9DBM_SETTING   	(0)
#define TX_POWER_POS0DBM_SETTING    (1)
#define TX_POWER_POS10DBM_SETTING   (2)
#define TX_POWER_POS22DBM_SETTING  	(3)

#define TX_POWER_FIRST_OPTION 		(TX_POWER_NEG9DBM_SETTING)
#define TX_POWER_LAST_OPTION 		(TX_POWER_POS22DBM_SETTING)



//LCD BL Levels
#define BL_LEVEL_OFF_SETTING   		(0)
#define BL_LEVEL_LOW_AUTO_SETTING	(1)
#define BL_LEVEL_MID_AUTO_SETTING	(2)
#define BL_LEVEL_HIGH_AUTO_SETTING	(3)
#define BL_LEVEL_LOW_SETTING    	(4)
#define BL_LEVEL_MID_SETTING   		(5)
#define BL_LEVEL_HIGH_SETTING  		(6)

#define BL_LEVEL_FIRST_OPTION 		(BL_LEVEL_OFF_SETTING)
#define BL_LEVEL_LAST_OPTION 		(BL_LEVEL_HIGH_SETTING)



//FREQ
#define FREQ_CHANNEL_FIRST  (1)
#define FREQ_CHANNEL_LAST   (8)	//see radio.c



//ID
#define DEVICE_ID_FIRST_SYMBOL      ('A')
#define DEVICE_ID_LAST_SYMBOL       ('Z')



//TIMEOUT
#define TIMEOUT_THRESHOLD_MIN 	(0)
#define TIMEOUT_THRESHOLD_MAX 	(3600)
#define TIMEOUT_THRESHOLD_STEP 	(10)



//FENCE
#define FENCE_THRESHOLD_MIN 	(0)
#define FENCE_THRESHOLD_MAX 	(5000)
#define FENCE_THRESHOLD_STEP 	(10)



//Structure with settings
struct settings_struct
{
    uint8_t device_number;              //this device number in group, 1...DEVICES_IN_GROUP

    uint8_t devices_on_air;				//total number of devices on air, 1...DEVICES_IN_GROUP

    char device_id;      				//user id, ASCII symbol

    uint8_t freq_channel;               //frequency tx/rx channel, LPD #1-69

    uint8_t tx_power_opt;               //tx power option, not an actual value

    uint8_t update_interval_opt;		//update interval option, not an actual value

    uint16_t timeout_threshold;        	//timeout treshold in seconds, unsigned. if it == 0, then timeout alarm not trigger (but, anyway, timeout is counting). See TIMEOUT_ALARM_DISABLED

    uint16_t fence_threshold;        	//fence treshold in meters, unsigned. if it == 0, then fence alarm not trigger. See FENCE_ALARM_DISABLED

    int8_t time_zone_dir;				//can be -1 or 1

    int8_t time_zone_hour;				//can be 0 ... 14 if time_zone_dir = 1; and 0 ... 12 if time_zone_dir = -1

    int8_t time_zone_minute;			//can be 0, 15, 30, 45

    uint8_t bl_level_opt;				//LCD backlight level option, not an actual value

    int16_t magn_offset_x;				//magnetometer offset X for hard iron compensation

    int16_t magn_offset_y;				//magnetometer offset Y for hard iron compensation

	union
	{
		float as_float;         		//magnetometer scale X for soft iron compensation
		uint16_t as_array[2];
	} magn_scale_x;

	union
	{
		float as_float;         		//magnetometer scale Y for soft iron compensation
		uint16_t as_array[2];
	} magn_scale_y;
};



struct settings_struct *get_settings(void);
uint8_t *get_update_interval_values(void);
int8_t *get_tx_power_values(void);
uint16_t *get_bl_level_values(void);
void settings_save_default(void);
void settings_load(void);
void settings_save(struct settings_struct *settings);



#endif /*SETTINGS_HEADER*/
