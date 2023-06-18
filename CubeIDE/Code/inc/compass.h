/*
	SPOKE

    file: compass.h
*/



#ifndef COMPASS_HEADER
#define COMPASS_HEADER

void init_compass(void);
void calibrate_compass(void);
void read_north(void);
uint8_t is_north_ready(void);
float get_north(void);

#endif /*COMPASS_HEADER*/
