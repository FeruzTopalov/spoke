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
void i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data);
uint8_t i2c_read(uint8_t i2c_addr, uint8_t reg_addr);
void i2c_read_multiple(uint8_t i2c_addr, uint8_t reg_addr, uint8_t size, uint8_t *buffer);



#endif /*I2C_HEADER*/
