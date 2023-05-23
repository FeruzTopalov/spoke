/*
    SPOKE

    file: settings.h
*/



#ifndef SETTINGS_HEADER
#define SETTINGS_HEADER



#define TIMEOUT_ALARM_DISABLED		(0)
#define FENCE_ALARM_DISABLED		(0)



//Send interval settings
#define UPDATE_INTERVAL_1S_SETTING		(0)
#define UPDATE_INTERVAL_5S_SETTING		(1)
#define UPDATE_INTERVAL_10S_SETTING		(2)
#define UPDATE_INTERVAL_30S_SETTING		(3)
#define UPDATE_INTERVAL_60S_SETTING		(4)

#define UPDATE_INTERVAL_FIRST_OPTION 	(UPDATE_INTERVAL_1S_SETTING)
#define UPDATE_INTERVAL_LAST_OPTION 	(UPDATE_INTERVAL_60S_SETTING)


//POWER SETTINGS
#define TX_POWER_1MILLIW_SETTING   	(0)
#define TX_POWER_10MILLIW_SETTING   (1)
#define TX_POWER_50MILLIW_SETTING   (2)
#define TX_POWER_100MILLIW_SETTING  (3)

#define TX_POWER_FIRST_OPTION 		(TX_POWER_1MILLIW_SETTING)
#define TX_POWER_LAST_OPTION 		(TX_POWER_100MILLIW_SETTING)



//FREQ
#define FREQ_CHANNEL_FIRST  (1)
#define FREQ_CHANNEL_LAST   (69)



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
};



struct settings_struct *get_settings(void);
uint8_t *get_update_interval_values(void);
uint8_t *get_tx_power_values(void);
void settings_save_default(void);
void settings_load(void);
void settings_save(struct settings_struct *settings);



#endif /*SETTINGS_HEADER*/
