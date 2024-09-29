/*
    SPOKE

    file: compass.c
*/



#include "stm32f10x.h"
#include <string.h>
#include <math.h>
#include "i2c.h"
#include "compass.h"
#include "sensors.h"
#include "service.h"
#include "lcd.h"
#include "settings.h"
#include "gpio.h"
#include "gps.h"
#include "lrns.h"
#include "menu.h"



#define GPS_SPEED_THRS				(2)	//threshold value for GPS speed in km/h to show true course
#define COMP_CAL_BUF_MAX_LEN 		(100)
#define COMP_CAL_STOP_TOL 			(50)	//tolerance on auto-stop condition



struct acc_data *p_acceleration;
struct mag_data *p_magnetic_field;
struct settings_struct *p_settings;
struct settings_struct settings_copy;
struct gps_num_struct *p_gps_num;
float north; //calculated north, +-pi
uint8_t gps_course = 0; //gps course is used instead of magnet course
uint8_t north_ready = 0; //flag is north value ready to readout
uint8_t last_is_horizontal = 0;



int16_t cal_buf_x[COMP_CAL_BUF_MAX_LEN];
int16_t cal_buf_y[COMP_CAL_BUF_MAX_LEN];
uint16_t cal_buf_len;

int16_t cal_x_start;
int16_t cal_y_start;
int16_t cal_x_max;
int16_t cal_x_min;
int16_t cal_y_max;
int16_t cal_y_min;

int16_t cal_pos_max;
int16_t cal_neg_max;
int16_t cal_abs_max;

float cal_plot_scale;

int16_t cal_offset_x;
int16_t cal_offset_y;
float cal_scale_x;
float cal_scale_y;



void init_compass(void)
{
	init_accelerometer();
	init_magnetometer();
	p_acceleration = get_acceleration();
	p_magnetic_field = get_magnetic_field();
	p_gps_num = get_gps_num();
	p_settings = get_settings();
	settings_copy = *p_settings;
}



void init_compass_calibration(void)
{
	memset(cal_buf_x, 0, 2 * COMP_CAL_BUF_MAX_LEN);
	memset(cal_buf_y, 0, 2 * COMP_CAL_BUF_MAX_LEN);
	cal_buf_len = 0;
	cal_x_start = 0;
	cal_y_start = 0;
	cal_x_max = 0;
	cal_x_min = 0;
	cal_y_max = 0;
	cal_y_min = 0;
	cal_pos_max = 0;
	cal_neg_max = 0;
	cal_abs_max = 0;
	cal_plot_scale = 0;
	cal_offset_x = 0;
	cal_offset_y = 0;
	cal_scale_x = 0;
	cal_scale_y = 0;
}



uint8_t calibrate_compass_new(void)
{
	if ((gps_course == 0) && (north_ready == 1))	//only if magn was read in read_compass()
	{
		cal_buf_x[cal_buf_len] = limit_to(p_magnetic_field->mag_x.as_integer, 2047, -2048);
		cal_buf_y[cal_buf_len] = limit_to(p_magnetic_field->mag_y.as_integer, 2047, -2048);

		if (cal_buf_len == 0)		//only for the first time
		{
			cal_x_start = cal_x_max = cal_x_min = cal_buf_x[0];
			cal_y_start = cal_y_max = cal_y_min = cal_buf_y[0];
		}

		//find max/min
		cal_x_max = maxv(cal_buf_x[cal_buf_len], cal_x_max);
		cal_x_min = minv(cal_buf_x[cal_buf_len], cal_x_min);
		cal_y_max = maxv(cal_buf_y[cal_buf_len], cal_y_max);
		cal_y_min = minv(cal_buf_y[cal_buf_len], cal_y_min);

		//find abs max
		cal_pos_max = maxv(absv(cal_x_max), absv(cal_y_max));
		cal_neg_max = maxv(absv(cal_x_min), absv(cal_y_min));
		cal_abs_max = maxv(cal_pos_max, cal_neg_max);
		cal_plot_scale = (float)(LCD_SIZE_Y / 2)/cal_abs_max;

		//auto-stop if we reached initial values (i.e. completed turnaround)
		if (cal_buf_len > (COMP_CAL_BUF_MAX_LEN / 2)) //only if we already acquired more than a half of buffer
		{
			int16_t cal_diff_x, cal_diff_y;
			cal_diff_x = absv(cal_buf_x[cal_buf_len] - cal_x_start);
			cal_diff_y = absv(cal_buf_y[cal_buf_len] - cal_y_start);

			if ((cal_diff_x < COMP_CAL_STOP_TOL) && (cal_diff_y < COMP_CAL_STOP_TOL))
			{
				return 0; //return 0 if end of calibration
			}
		}

		if (cal_buf_len == (COMP_CAL_BUF_MAX_LEN - 1)) //last point in the buffer
		{
			return 0; //return 0 if end of calibration when buffer ended
		}

		cal_buf_len++;	//increment before exit
	}

	return 1;	//return 1 if the calibration is ongoing
}



void compass_hard_soft_compensation(void)
{
	int16_t cal_avg_delta_x;
	int16_t cal_avg_delta_y;
	int16_t cal_avg_delta;

	//calc offset for hard iron
	cal_offset_x = (cal_x_max + cal_x_min) / 2;
	cal_offset_y = (cal_y_max + cal_y_min) / 2;

	//calc soft iron compensation (simple)
	cal_avg_delta_x = (cal_x_max - cal_x_min) / 2;
	cal_avg_delta_y = (cal_y_max - cal_y_min) / 2;
	cal_avg_delta = (cal_avg_delta_x + cal_avg_delta_y) / 2;

	cal_scale_x = (float)cal_avg_delta / cal_avg_delta_x;
	cal_scale_y = (float)cal_avg_delta / cal_avg_delta_y;

	//hard and soft compensation itself
	cal_x_max = cal_x_min = 0;
	cal_y_max = cal_y_min = 0;
	for (uint16_t pt = 0; pt < (cal_buf_len + 1); pt++)
	{
		cal_buf_x[pt] = (cal_buf_x[pt] - cal_offset_x) * cal_scale_x;
		cal_buf_y[pt] = (cal_buf_y[pt] - cal_offset_y) * cal_scale_y;

		//find max/min
		cal_x_max = maxv(cal_buf_x[pt], cal_x_max);
		cal_x_min = minv(cal_buf_x[pt], cal_x_min);
		cal_y_max = maxv(cal_buf_y[pt], cal_y_max);
		cal_y_min = minv(cal_buf_y[pt], cal_y_min);
	}

	//prepare for compensated print
	//find abs max
	cal_pos_max = maxv(absv(cal_x_max), absv(cal_y_max));
	cal_neg_max = maxv(absv(cal_x_min), absv(cal_y_min));
	cal_abs_max = maxv(cal_pos_max, cal_neg_max);
	cal_plot_scale = (float)(LCD_SIZE_Y / 2)/cal_abs_max;
}



void compass_calibration_save()
{
    //save calibration in settings
    settings_copy.magn_offset_x = cal_offset_x;
    settings_copy.magn_offset_y = cal_offset_y;
    settings_copy.magn_scale_x.as_float = cal_scale_x;
    settings_copy.magn_scale_y.as_float = cal_scale_y;
    settings_save(&settings_copy);
    settings_load();
}



uint8_t read_compass(void)
{
	float comp_x, comp_y;
	uint8_t current_is_horizontal;

	current_is_horizontal = is_horizontal();

	if (current_is_horizontal)	//if the device is oriented horizontally
	{
		read_magn();

		comp_x = (p_magnetic_field->mag_x.as_integer - p_settings->magn_offset_x) * p_settings->magn_scale_x.as_float;
		comp_y = (p_magnetic_field->mag_y.as_integer - p_settings->magn_offset_y) * p_settings->magn_scale_y.as_float;

		north = atan2(-comp_x, comp_y);		//from atan2(y, x) to atan2(-x, y) to rotate result pi/2 CCW

		gps_course = 0;
		north_ready = 1;

		last_is_horizontal = current_is_horizontal;
		return 1; //return 1 if horizontal
	}
	else	//otherwise use GPS course
	{
		if (p_gps_num->speed > GPS_SPEED_THRS)	//only when moving
		{
			north = p_gps_num->course * deg_to_rad;
			gps_course = 1;
			north_ready = 1;
		}
		else
		{
			north_ready = 0;
		}

		if (last_is_horizontal == 1)
		{
			last_is_horizontal = current_is_horizontal;
			return 1; //in order to update the LCD last time and, potentially, hide the compass arrow; i.e. fix the arrow freeze after going from horizontal
		}
		else
		{
			last_is_horizontal = current_is_horizontal;
			return 0; //return 0 if not horizontal
		}

	}
}



uint8_t is_north_ready(void)
{
	return north_ready;
}



uint8_t is_gps_course(void)
{
	return gps_course;
}



float get_north(void)
{
	return north;
}



uint16_t get_cal_buf_len(void)
{
	return cal_buf_len;
}



int16_t *get_cal_buf_x(void)
{
	return &cal_buf_x[0];
}



int16_t *get_cal_buf_y(void)
{
	return &cal_buf_y[0];
}



float get_cal_plot_scale(void)
{
	return cal_plot_scale;
}



