/*
    SPOKE

    file: compass.c
*/



#include "stm32f10x.h"
#include "i2c.h"
#include "compass.h"
#include "sensors.h"
#include "service.h"
#include "lcd.h"



struct acc_data *p_acceleration;
struct mag_data *p_magnetic_field;
char buf[15];



void init_compass(void)
{
	if (init_accelerometer() && init_magnetometer())
	{
		p_acceleration = get_acceleration();
		p_magnetic_field = get_magnetic_field();

		#define BUF_LEN (360)
		int16_t buf_mag_x[BUF_LEN] = {0};
		int16_t buf_mag_y[BUF_LEN] = {0};
		uint16_t buf_pointer = 0;

		int16_t abs_x_max = 0;
		int16_t abs_y_max = 0;

		while (1)
		{
			lcd_clear();

			//print buf_pointer
			itoa32(buf_pointer, buf);
			lcd_print(0, 0, buf, 0);

			//read magnetometr
			read_magn();

			//limit values
			if (p_magnetic_field->mag_x.as_integer > 2047)
			{
				p_magnetic_field->mag_x.as_integer = 2047;
			}
			else if (p_magnetic_field->mag_x.as_integer < -2048)
			{
				p_magnetic_field->mag_x.as_integer = -2048;
			}

			if (p_magnetic_field->mag_y.as_integer > 2047)
			{
				p_magnetic_field->mag_y.as_integer = 2047;
			}
			else if (p_magnetic_field->mag_y.as_integer < -2048)
			{
				p_magnetic_field->mag_y.as_integer = -2048;
			}

			//find abs max
			int8_t sign_x = 1;
			if (p_magnetic_field->mag_x.as_integer < 0)
			{
				sign_x = -1;
			}

			if (p_magnetic_field->mag_x.as_integer * sign_x > abs_x_max)
			{
				abs_x_max = p_magnetic_field->mag_x.as_integer * sign_x;
			}

			int8_t sign_y = 1;
			if (p_magnetic_field->mag_y.as_integer < 0)
			{
				sign_y = -1;
			}

			if (p_magnetic_field->mag_y.as_integer * sign_y > abs_y_max)
			{
				abs_y_max = p_magnetic_field->mag_y.as_integer * sign_y;
			}

			//store values
			buf_mag_x[buf_pointer] = p_magnetic_field->mag_x.as_integer;
			buf_mag_y[buf_pointer] = p_magnetic_field->mag_y.as_integer;

			if (buf_pointer == (BUF_LEN - 1))
			{
				buf_pointer = 0;
			}
			else
			{
				buf_pointer++;
			}

			//draw
			uint8_t x_dot, y_dot;
			float scale;

			if (abs_x_max > abs_y_max)
			{
				scale = (float)32/abs_x_max;
			}
			else
			{
				scale = (float)32/abs_y_max;
			}

			for (uint16_t i = 0; i < BUF_LEN; i++)
			{
				x_dot = (uint8_t)((float)buf_mag_x[i] * scale + 64);
				y_dot = (uint8_t)((float)buf_mag_y[i] * scale + 32);
				lcd_pixel(x_dot, y_dot, 1);
			}

			//view
			lcd_update();
			delay_cyc(5000);
		}
	}
}
