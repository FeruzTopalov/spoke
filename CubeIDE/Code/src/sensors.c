/*
    SPOKE

    file: sensors.c
*/



#include "stm32f10x.h"
#include "i2c.h"
#include "sensors.h"

#define LSM303_ADDR_ACC         	(0x32)		//write address
#define LSM303_ADDR_MAG          	(0x3C)		//write address

//acc reg addr
#define		LSM303_REG_ACC_CTRL_REG1_A         (0x20)
#define		LSM303_REG_ACC_CTRL_REG2_A         (0x21)
#define		LSM303_REG_ACC_CTRL_REG3_A         (0x22)
#define		LSM303_REG_ACC_CTRL_REG4_A         (0x23)
#define		LSM303_REG_ACC_CTRL_REG5_A         (0x24)
#define		LSM303_REG_ACC_CTRL_REG6_A         (0x25)
#define		LSM303_REG_ACC_REFERENCE_A         (0x26)
#define		LSM303_REG_ACC_STATUS_REG_A        (0x27)
#define		LSM303_REG_ACC_OUT_X_L_A           (0x28)
#define		LSM303_REG_ACC_OUT_X_H_A           (0x29)
#define		LSM303_REG_ACC_OUT_Y_L_A           (0x2A)
#define		LSM303_REG_ACC_OUT_Y_H_A           (0x2B)
#define		LSM303_REG_ACC_OUT_Z_L_A           (0x2C)
#define		LSM303_REG_ACC_OUT_Z_H_A           (0x2D)
#define		LSM303_REG_ACC_FIFO_CTRL_REG_A     (0x2E)
#define		LSM303_REG_ACC_FIFO_SRC_REG_A      (0x2F)
#define		LSM303_REG_ACC_INT1_CFG_A          (0x30)
#define		LSM303_REG_ACC_INT1_SOURCE_A       (0x31)
#define		LSM303_REG_ACC_INT1_THS_A          (0x32)
#define		LSM303_REG_ACC_INT1_DURATION_A     (0x33)
#define		LSM303_REG_ACC_INT2_CFG_A          (0x34)
#define		LSM303_REG_ACC_INT2_SOURCE_A       (0x35)
#define		LSM303_REG_ACC_INT2_THS_A          (0x36)
#define		LSM303_REG_ACC_INT2_DURATION_A     (0x37)
#define		LSM303_REG_ACC_CLICK_CFG_A         (0x38)
#define		LSM303_REG_ACC_CLICK_SRC_A         (0x39)
#define		LSM303_REG_ACC_CLICK_THS_A         (0x3A)
#define		LSM303_REG_ACC_TIME_LIMIT_A        (0x3B)
#define		LSM303_REG_ACC_TIME_LATENCY_A      (0x3C)
#define		LSM303_REG_ACC_TIME_WINDOW_A       (0x3D)

//acc reg val
#define LSM303_ACC_CTRL_REG1_A__100ODR_NORM_XYZ     	(0b01010111)	// CTRL_REG1_A (0x20): ODR = 100 Hz, normal mode, XYZ enabled
#define LSM303_ACC_CTRL_REG2_A__HPF         			(0b00110001)	// CTRL_REG2_A (0x21): high-pass for INT1
#define LSM303_ACC_CTRL_REG3_A__I1_AOI1             	(0b01000000)	// CTRL_REG3_A (0x22): interrupt on INT1 pin)
#define LSM303_ACC_CTRL_REG4_A__FS2G_HR             	(0b00001000)	// CTRL_REG4_A (0x23): FS=±2g (00), HR=1 (high resolution)
#define LSM303_ACC_CTRL_REG5_A__LIR_INT1            	(0b00001000)	// CTRL_REG5_A (0x24): LIR_INT1=1 (latched interrupt on INT1)
#define LSM303_ACC_CTRL_REG6_A__H_LACTIVE           	(0b00000010)	// CTRL_REG6_A (0x25): H_LACTIVE=1 (INT1 active low polarity)
#define LSM303_ACC_INT1_CFG_A__MOVEMENT_HIGH_XYZ    	(0b00101010)	// INT1_CFG_A (0x30): AOI=0, 6D=0, XYZ high events
#define LSM303_ACC_INT1_THS_A							(10)			// INT1_THS_A (0x32): INT1 threshold, 0-127 of the full scale FS

//mag reg addr
#define		LSM303_REG_MAG_CRA_REG_M            (0x00)
#define		LSM303_REG_MAG_CRB_REG_M            (0x01)
#define		LSM303_REG_MAG_MR_REG_M             (0x02)
#define		LSM303_REG_MAG_OUT_X_H_M            (0x03)
#define		LSM303_REG_MAG_OUT_X_L_M            (0x04)
#define		LSM303_REG_MAG_OUT_Z_H_M            (0x05)
#define		LSM303_REG_MAG_OUT_Z_L_M            (0x06)
#define		LSM303_REG_MAG_OUT_Y_H_M            (0x07)
#define		LSM303_REG_MAG_OUT_Y_L_M            (0x08)
#define		LSM303_REG_MAG_SR_REG_Mg            (0x09)
#define		LSM303_REG_MAG_IRA_REG_M            (0x0A)
#define		LSM303_REG_MAG_IRB_REG_M            (0x0B)
#define		LSM303_REG_MAG_IRC_REG_M            (0x0C)
#define		LSM303_REG_MAG_TEMP_OUT_H_M         (0x31)
#define		LSM303_REG_MAG_TEMP_OUT_L_M         (0x32)

//mag reg val
#define		LSM303_MAG_CRA_REG_M__75HZ        	(0x18)
#define		LSM303_MAG_CRB_REG_M__13GS			(0x20)
#define		LSM303_MAG_CRB_REG_M__40GS       	(0x80)
#define		LSM303_MAG_MR_REG_M__CONT         	(0x00)
#define		LSM303_MAG_MR_REG_M__SLEEP        	(0x03)



#define ACC_HORIZ_THRS	(900)	//threshold value for Z axis when checking horizontality
#define ACC_KALMAN_K 	(0.15)	//Kalman filter coefficient for accelerometer
#define MAG_KALMAN_K 	(0.3)	//Kalman filter coefficient for magnetometer



struct acc_data acceleration;
struct mag_data magnetic_field;



float acc_k = ACC_KALMAN_K;
float mag_k = MAG_KALMAN_K;



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

	//Kalman filtration of Z axis because it is only used
	acceleration.acc_z.as_integer = (int16_t)(acc_k * acceleration.acc_z.as_integer + (1 - acc_k) * acc_z_prev);
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
	magnetic_field.mag_x.as_integer = (int16_t)(mag_k * magnetic_field.mag_x.as_integer + (1 - mag_k) * mag_x_prev);
	magnetic_field.mag_y.as_integer = (int16_t)(mag_k * magnetic_field.mag_y.as_integer + (1 - mag_k) * mag_y_prev);
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



struct acc_data *get_acceleration(void)
{
	return &acceleration;
}



struct mag_data *get_magnetic_field(void)
{
	return &magnetic_field;
}
