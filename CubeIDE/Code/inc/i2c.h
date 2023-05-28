/*
    SPOKE
    
    file: i2c.h
*/



#ifndef I2C_HEADER
#define I2C_HEADER



void i2c_init(void);
void i2c_clock_disable(void);
void i2c_clock_enable(void);
uint8_t i2c_poll(uint8_t i2c_addr);



#endif /*I2C_HEADER*/
