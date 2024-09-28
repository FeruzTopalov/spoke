/*
	SPOKE

    file: compass.h
*/



#ifndef COMPASS_HEADER
#define COMPASS_HEADER

void init_compass(void);
void calibrate_compass(void);
uint8_t read_compass(void);
uint8_t is_north_ready(void);
uint8_t is_gps_course(void);
float get_north(void);

#endif /*COMPASS_HEADER*/
