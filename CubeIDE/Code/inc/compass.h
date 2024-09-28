/*
	SPOKE

    file: compass.h
*/



#ifndef COMPASS_HEADER
#define COMPASS_HEADER

void init_compass(void);
void init_compass_calibration(void);
void calibrate_compass_new(void);
void calibrate_compass(void);
uint8_t read_compass(void);
uint8_t is_north_ready(void);
uint8_t is_gps_course(void);
float get_north(void);
int16_t get_cal_buf_len(void);

#endif /*COMPASS_HEADER*/
