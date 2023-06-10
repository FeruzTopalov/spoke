/*
    SPOKE

    file: compass.c
*/



#include "stm32f10x.h"
#include <math.h>
#include "i2c.h"
#include "compass.h"
#include "sensors.h"
#include "service.h"
#include "lcd.h"



struct acc_data *p_acceleration;
struct mag_data *p_magnetic_field;



void init_compass(void)
{
	init_accelerometer();
	init_magnetometer();
	p_acceleration = get_acceleration();
	p_magnetic_field = get_magnetic_field();

	//start calibration if requested
	if (!((GPIOB->IDR) & GPIO_IDR_IDR3))	//if DOWN button was pressed upon power up
	{
		lcd_clear();
		lcd_print(0, 3, "Compass cal", 0);
		lcd_print(1, 0, "- hold horizont.", 0);
		lcd_print(2, 0, "- click DOWN", 0);
		lcd_print(3, 0, "- turnaround 360", 0);
		lcd_update();

		while (!((GPIOB->IDR) & GPIO_IDR_IDR3)){}	//wait for user release DOWN/ESC after power up
		delay_cyc(300000);	//hold on for a moment
		while ((GPIOB->IDR) & GPIO_IDR_IDR3){}		//wait for DOWN/ESC click to start cal

		calibrate_compass();
	}
}



void calibrate_compass(void)
{
	#define LCD_CNTR_X (64)
	#define LCD_CNTR_Y (32)

	#define BUF_LEN (180)
	uint16_t len_tot = 0;
	int16_t buf_x[BUF_LEN] = {0};
	int16_t buf_y[BUF_LEN] = {0};

	#define STOP_TOL (30)	//tolerance on auto-stop condition
	int16_t x_start = 0;
	int16_t y_start = 0;
	int16_t x_max = 0;
	int16_t x_min = 0;
	int16_t y_max = 0;
	int16_t y_min = 0;

	int16_t pos_max = 0;
	int16_t neg_max = 0;
	int16_t abs_max = 0;

	float plot_scale = 0;
	char buf[15];	//for lcd prints

	//begin
	lcd_clear();
	lcd_print(0, 0, "start", 0);
	lcd_update();

	//init start value and min/max values
	read_magn();
	x_start = x_max = x_min = p_magnetic_field->mag_x.as_integer;
	y_start = y_max = y_min = p_magnetic_field->mag_y.as_integer;

	//acquire and plot magnetometer values during turnaround
	for (uint16_t pt = 0; pt < BUF_LEN; pt++)
	{
		lcd_clear();
		lcd_pixel(LCD_CNTR_X, LCD_CNTR_Y, 1);	//plot a dot in lcd center

		//print pointer
		itoa32(pt, buf);
		lcd_print(0, 0, buf, 0);

		//read magnetometr
		read_magn();

		//limit and store values
		buf_x[pt] = limit_to(p_magnetic_field->mag_x.as_integer, 2047, -2048);
		buf_y[pt] = limit_to(p_magnetic_field->mag_y.as_integer, 2047, -2048);

		//find max/min
		x_max = maxv(p_magnetic_field->mag_x.as_integer, x_max);
		x_min = minv(p_magnetic_field->mag_x.as_integer, x_min);
		y_max = maxv(p_magnetic_field->mag_y.as_integer, y_max);
		y_min = minv(p_magnetic_field->mag_y.as_integer, y_min);

		//find abs max
		pos_max = maxv(absv(x_max), absv(y_max));
		neg_max = maxv(absv(x_min), absv(y_min));
		abs_max = maxv(pos_max, neg_max);
		plot_scale = (float)32/abs_max;

		//draw
		for (uint16_t i = 0; i < BUF_LEN; i++)
		{
			uint8_t x_dot, y_dot;
			x_dot = (uint8_t)((float)buf_x[i] * plot_scale + LCD_CNTR_X);
			y_dot = (uint8_t)((float)buf_y[i] * plot_scale + LCD_CNTR_Y);
			lcd_set_pixel(x_dot, y_dot);
		}

		//view
		lcd_update();
		delay_cyc(5000);

		//auto-stop if we reached initial values (i.e. completed turnaround)
		if (pt > (BUF_LEN / 2)) //only if we already acquired more than a half of buffer
		{
			int16_t diff_x, diff_y;
			diff_x = absv(buf_x[pt] - x_start);
			diff_y = absv(buf_y[pt] - y_start);

			if ((diff_x < STOP_TOL) && (diff_y < STOP_TOL))
			{
				len_tot = pt + 1;	//save number of points
				break; //exit for loop
			}
		}
	}

	if (len_tot == 0)	//if no auto-stop
	{
		len_tot = BUF_LEN;	//if stopped after for loop save max val as BUF_LEN
	}

	//calc offset for hard iron
	int16_t offset_x;
	int16_t offset_y;
	offset_x = (x_max + x_min) / 2;
	offset_y = (y_max + y_min) / 2;

	//calc soft iron compensation (simple)
	int16_t avg_delta_x, avg_delta_y;
	avg_delta_x = (x_max - x_min) / 2;
	avg_delta_y = (y_max - y_min) / 2;

	int16_t avg_delta;
	avg_delta = (avg_delta_x + avg_delta_y) / 2;

	float scale_x, scale_y;
	scale_x = (float)avg_delta / avg_delta_x;
	scale_y = (float)avg_delta / avg_delta_y;

	//hard and soft compensation itself
	x_max = x_min = 0;
	y_max = y_min = 0;
	for (uint16_t pt = 0; pt < len_tot; pt++)
	{
		buf_x[pt] = (buf_x[pt] - offset_x) * scale_x;
		buf_y[pt] = (buf_y[pt] - offset_y) * scale_y;

		//find max/min
		x_max = maxv(buf_x[pt], x_max);
		x_min = minv(buf_x[pt], x_min);
		y_max = maxv(buf_y[pt], y_max);
		y_min = minv(buf_y[pt], y_min);
	}

	//prepare for compensated print
	pos_max = maxv(absv(x_max), absv(y_max));
	neg_max = maxv(absv(x_min), absv(y_min));
	abs_max = maxv(pos_max, neg_max);
	plot_scale = (float)32/abs_max;

	//draw result after calibration
	lcd_clear();
	lcd_pixel(LCD_CNTR_X, LCD_CNTR_Y, 1);	//plot a dot in lcd center
	for (uint16_t i = 0; i < len_tot; i++)
	{
		uint8_t x_dot, y_dot;
		x_dot = (uint8_t)((float)buf_x[i] * plot_scale + LCD_CNTR_X);
		y_dot = (uint8_t)((float)buf_y[i] * plot_scale + LCD_CNTR_Y);
		lcd_set_pixel(x_dot, y_dot);
	}
	lcd_update();



	while (1){}
}
