/*
    SPOKE

    file: sensors.c
*/



#include "stm32f10x.h"
#include "i2c.h"
#include "sensors.h"
#include "lsm303dlhc.h"



#define ACC_HORIZ_THRS	(900)	//threshold value for Z axis when checking horizontality
#define ACC_KALMAN_K 	(0.15)	//Kalman filter coefficient for accelerometer
#define MAG_KALMAN_K 	(0.4)	//Kalman filter coefficient for magnetometer



struct acc_data acceleration;
struct mag_data magnetic_field;

float acc_k = ACC_KALMAN_K;
float mag_k = MAG_KALMAN_K;


uint8_t acc_first_run = 0;	//flags to indicate first run in order to properly initialize previous sensor value for Kalman filtering
uint8_t mag_first_run = 0;



uint8_t init_accelerometer(void)	//always enabled for movement detection
{
	if (i2c_poll(LSM303_ADDR_ACC))
	{
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG2_A, LSM303_ACC_CTRL_REG2_A__HPF);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG3_A, LSM303_ACC_CTRL_REG3_A__I1_AOI1);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG4_A, LSM303_ACC_CTRL_REG4_A__FS2G_HR);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG6_A, LSM303_ACC_CTRL_REG6_A__H_LACTIVE);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_INT1_CFG_A, LSM303_ACC_INT1_CFG_A__MOVEMENT_HIGH_XYZ);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_INT1_THS_A, LSM303_ACC_INT1_THS_A);
		i2c_write(LSM303_ADDR_ACC, LSM303_REG_ACC_CTRL_REG1_A, LSM303_ACC_CTRL_REG1_A__100ODR_NORM_XYZ);

		return 1;
	}
	else
	{
		return 0;
	}
}



uint8_t init_magnetometer(void)	//enabled only when compass is needed (in navigation and calibration menus)
{
	if (i2c_poll(LSM303_ADDR_MAG))
	{
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_CRA_REG_M, LSM303_MAG_CRA_REG_M__75HZ);
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_CRB_REG_M, LSM303_MAG_CRB_REG_M__40GS);
		i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_MR_REG_M, LSM303_MAG_MR_REG_M__SLEEP);
		return 1;
	}
	else
	{
		return 0;
	}
}



void start_magnetometer(void)
{
	i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_MR_REG_M, LSM303_MAG_MR_REG_M__CONT);
}



void stop_magnetometer(void)
{
	i2c_write(LSM303_ADDR_MAG, LSM303_REG_MAG_MR_REG_M, LSM303_MAG_MR_REG_M__SLEEP);
}



void read_accel(void)
{
	int16_t acc_z_prev;
	uint8_t buf[6];

	acc_z_prev = acceleration.acc_z.as_integer; //save for Kalman filter

	i2c_read_multiple(LSM303_ADDR_ACC, LSM303_REG_ACC_OUT_X_L_A, 6, buf);

	acceleration.acc_x.as_array[0] = buf[0];
	acceleration.acc_x.as_array[1] = buf[1];
	acceleration.acc_y.as_array[0] = buf[2];
	acceleration.acc_y.as_array[1] = buf[3];
	acceleration.acc_z.as_array[0] = buf[4];
	acceleration.acc_z.as_array[1] = buf[5];

	acceleration.acc_x.as_integer /= 16;	//signed shift right 4 bit, align 12 bit data in 16 bit wide type (HR bit must be 1 in CTRL_REG4_A to enable 12 bit result)
	acceleration.acc_y.as_integer /= 16;
	acceleration.acc_z.as_integer /= 16;

	acceleration.acc_y.as_integer *= -1;	//invert Y and Z due to physical location of the sensor on the PCB
	acceleration.acc_z.as_integer *= -1;	//after this correction and for the device in normal orientation: X - to the right, Y - forward, Z - up into face

	//Kalman filtration of Z axis because it is the only used
	if (acc_first_run == 1)
	{
		acc_first_run = 0; //clear after first run; no changes to acc value
	}
	else //not the first run, use prev value
	{
		acceleration.acc_z.as_integer = (int16_t)(acc_k * acceleration.acc_z.as_integer + (1 - acc_k) * acc_z_prev);
	}
}



void read_magn(void)
{
	int16_t mag_x_prev;
	int16_t mag_y_prev;
	uint8_t buf[6];

	mag_x_prev = magnetic_field.mag_x.as_integer;
	mag_y_prev = magnetic_field.mag_y.as_integer;

	i2c_read_multiple(LSM303_ADDR_MAG, LSM303_REG_MAG_OUT_X_H_M, 6, buf);

	magnetic_field.mag_x.as_array[1] = buf[0];
	magnetic_field.mag_x.as_array[0] = buf[1];
	magnetic_field.mag_z.as_array[1] = buf[2];
	magnetic_field.mag_z.as_array[0] = buf[3];
	magnetic_field.mag_y.as_array[1] = buf[4];
	magnetic_field.mag_y.as_array[0] = buf[5];

	magnetic_field.mag_z.as_integer *= -1;	//invert Y and Z due to physical location of the sensor on the PCB
	magnetic_field.mag_y.as_integer *= -1;	//after this correction and for the device in normal orientation: X - to the right, Y - forward, Z - up into face

	//Kalman filtration of X and Y axes because they are only used
	if (mag_first_run == 1)
	{
		mag_first_run = 0; //clear after first run; no changes to mag values
	}
	else //not the first run, use prev value
	{
		magnetic_field.mag_x.as_integer = (int16_t)(mag_k * magnetic_field.mag_x.as_integer + (1 - mag_k) * mag_x_prev);
		magnetic_field.mag_y.as_integer = (int16_t)(mag_k * magnetic_field.mag_y.as_integer + (1 - mag_k) * mag_y_prev);
	}
}



uint8_t is_horizontal(void)
{
	read_accel();
	if (acceleration.acc_z.as_integer > ACC_HORIZ_THRS)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



void sensors_mark_as_first_run(void)
{
	acc_first_run = 1;
	mag_first_run = 1;
}



struct acc_data *get_acceleration(void)
{
	return &acceleration;
}



struct mag_data *get_magnetic_field(void)
{
	return &magnetic_field;
}
