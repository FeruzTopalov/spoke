/*
    SPOKE

    file: sensors.c
*/



#include "stm32f10x.h"
#include "i2c.h"
#include "sensors.h"



struct acc_data acceleration;
struct mag_data magnetic_field;



uint8_t init_accelerometer(void)
{
	if (i2c_poll(LSM303_ADDR_ACC))
	{
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG1_A, LSM303_ACC_CTRL_REG1_A__100ODR_NORM_XYZ);
		return 1;
	}
	else
	{
		return 0;
	}
}



uint8_t init_magnetometer(void)
{
	if (i2c_poll(LSM303_ADDR_MAG))
	{
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_CRA_REG_M, LSM303_MAG_CRA_REG_M__75HZ);
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_CRB_REG_M, LSM303_MAG_CRB_REG_M__1_3GS);
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_MR_REG_M, LSM303_MAG_MR_REG_M__CONT);
		return 1;
	}
	else
	{
		return 0;
	}
}



void read_accel(void)
{
	uint8_t buf[6];
	i2c_read_multiple(LSM303_ADDR_ACC, LSM303_REG_ACC_OUT_X_L_A, 6, buf);

	acceleration.acc_x.as_array[0] = buf[0];
	acceleration.acc_x.as_array[1] = buf[1];
	acceleration.acc_y.as_array[0] = buf[2];
	acceleration.acc_y.as_array[1] = buf[3];
	acceleration.acc_z.as_array[0] = buf[4];
	acceleration.acc_z.as_array[1] = buf[5];

	acceleration.acc_x.as_integer /= 16;	//signed shift right 4 bit, align 12 bit data in 16 bit wide type
	acceleration.acc_y.as_integer /= 16;
	acceleration.acc_z.as_integer /= 16;

	acceleration.acc_y.as_integer *= -1;	//invert Y and Z due to physical location of the sensor on the PCB
	acceleration.acc_z.as_integer *= -1;	//after this correction and for the device in normal orientation: X - to the right, Y - forward, Z - up into face
}



void read_magn(void)
{
	uint8_t buf[6];
	i2c_read_multiple(LSM303_ADDR_MAG, LSM303_REG_MAG_OUT_X_H_M, 6, buf);

	magnetic_field.mag_x.as_array[1] = buf[0];
	magnetic_field.mag_x.as_array[0] = buf[1];
	magnetic_field.mag_z.as_array[1] = buf[2];
	magnetic_field.mag_z.as_array[0] = buf[3];
	magnetic_field.mag_y.as_array[1] = buf[4];
	magnetic_field.mag_y.as_array[0] = buf[5];

	magnetic_field.mag_z.as_integer *= -1;	//invert Y and Z due to physical location of the sensor on the PCB
	magnetic_field.mag_y.as_integer *= -1;	//after this correction and for the device in normal orientation: X - to the right, Y - forward, Z - up into face
}



struct acc_data *get_acceleration(void)
{
	return &acceleration;
}



struct mag_data *get_magnetic_field(void)
{
	return &magnetic_field;
}
