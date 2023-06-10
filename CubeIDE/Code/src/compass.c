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

	calibrate_compass();
}



void calibrate_compass(void)
{
	char buf[15];	//for lcd prints

	lcd_clear();
	lcd_print(0, 3, "Compass cal", 0);
	lcd_print(1, 0, "- hold horizont.", 0);
	lcd_print(2, 0, "- click DOWN", 0);
	lcd_print(3, 0, "- turnaround 360", 0);
	lcd_update();

	while ((GPIOB->IDR) & GPIO_IDR_IDR3){}

	lcd_clear();
	lcd_print(0, 0, "start", 0);
	lcd_update();

	while (1){}
}
