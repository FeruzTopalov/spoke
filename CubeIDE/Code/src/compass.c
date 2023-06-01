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

		while (1)
		{
			lcd_clear();



			read_accel();
			read_magn();



			lcd_print(0, 0, "ACC", 0);

			lcd_print(1, 0, "X", 0);
			itoa32(p_acceleration->acc_x.as_integer, buf);
			lcd_print(1, 2, buf, 0);

			lcd_print(2, 0, "Y", 0);
			itoa32(p_acceleration->acc_y.as_integer, buf);
			lcd_print(2, 2, buf, 0);

			lcd_print(3, 0, "Z", 0);
			itoa32(p_acceleration->acc_z.as_integer, buf);
			lcd_print(3, 2, buf, 0);



			lcd_print(0, 8, "MAG", 0);

			lcd_print(1, 8, "X", 0);
			itoa32(p_magnetic_field->mag_x.as_integer, buf);
			lcd_print(1, 10, buf, 0);

			lcd_print(2, 8, "Y", 0);
			itoa32(p_magnetic_field->mag_y.as_integer, buf);
			lcd_print(2, 10, buf, 0);

			lcd_print(3, 8, "Z", 0);
			itoa32(p_magnetic_field->mag_z.as_integer, buf);
			lcd_print(3, 10, buf, 0);



			lcd_update();
			delay_cyc(100000);
		}
	}
}
