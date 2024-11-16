/*
	SPOKE

    file: sensors.h
*/



#ifndef SENSORS_HEADER
#define SENSORS_HEADER



uint8_t init_accelerometer(void);
uint8_t init_magnetometer(void);
void start_accelerometer(void);
void stop_accelerometer(void);
void start_magnetometer(void);
void stop_magnetometer(void);
void read_accel(void);
void read_magn(void);
uint8_t is_horizontal(void);
struct acc_data *get_acceleration(void);
struct mag_data *get_magnetic_field(void);



struct acc_data
{
	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} acc_x;

	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} acc_y;

	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} acc_z;
};



struct mag_data
{
	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} mag_x;

	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} mag_y;

	union
	{
		int16_t as_integer;         //two's complement
		uint8_t as_array[2];
	} mag_z;
};



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
#define		LSM303_ACC_CTRL_REG1_A__100ODR_NORM_XYZ		(0x57)
#define		LSM303_ACC_CTRL_REG1_A__SLEEP_NORM_XYZ		(0x07)



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
#define		LSM303_MAG_CRA_REG_M__75HZ        (0x18)
#define		LSM303_MAG_CRB_REG_M__1_3GS       (0x20)
#define		LSM303_MAG_MR_REG_M__CONT         (0x00)
#define		LSM303_MAG_MR_REG_M__SLEEP        (0x03)


#endif /*SENSORS_HEADER*/
