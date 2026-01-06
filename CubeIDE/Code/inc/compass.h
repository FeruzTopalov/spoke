/*
	SPOKE

    file: compass.h
*/



#ifndef COMPASS_HEADER
#define COMPASS_HEADER

#define COMPASS_IS_NOT_AVAILABLE	(0)
#define COMPASS_IS_AVAILABLE		(1)

void init_compass(void);
void init_compass_calibration(void);
uint8_t calibrate_compass_new(void);
void compass_hard_soft_compensation(void);
void compass_calibration_save();
void calibrate_compass(void);
void start_compass(void);
void stop_compass(void);
uint8_t read_compass(void);
uint8_t is_north_ready(void);
uint8_t is_gps_course(void);
float get_north(void);
uint16_t get_cal_buf_len(void);
int16_t *get_cal_buf_x(void);
int16_t *get_cal_buf_y(void);
float get_cal_plot_scale(void);
uint8_t get_acc_availability(void);
uint8_t get_mag_availability(void);
uint8_t get_compass_availability(void);

#endif /*COMPASS_HEADER*/
