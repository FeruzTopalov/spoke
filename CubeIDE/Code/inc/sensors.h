/*
	SPOKE

    file: sensors.h
*/



#ifndef SENSORS_HEADER
#define SENSORS_HEADER



uint8_t init_accelerometer(void);
uint8_t init_magnetometer(void);
void start_magnetometer(void);
void stop_magnetometer(void);
void read_accel(void);
void read_magn(void);
uint8_t is_horizontal(void);
void sensors_mark_as_first_run(void);
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



#endif /*SENSORS_HEADER*/
