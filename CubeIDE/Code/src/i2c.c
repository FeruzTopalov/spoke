/*
    SPOKE
    
    file: i2c.c
*/



#include "stm32f10x.h"
#include "i2c.h"
#include "service.h"
#include "bit_band.h"



void i2c_init(void)
{
    //I2C config
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;     //enable i2c clock
    
    I2C1->CR2 &= ~I2C_CR2_FREQ;             //clear bits before set
    I2C1->CR2 |= I2C_CR2_FREQ_1;            //2 MHz periph clock
    
    //Stndard mode
    I2C1->CCR |= (uint16_t)15;              //CCR = TSCL/(2 * TPCLK1); TSCL = 1 / 100kHz standard mode freq; TPCLK1 = 1 / APB1 clock 3 MHz
    I2C1->TRISE |= (uint16_t)4;             //TRISE = (Tr max/TPCLK1)+1; Tr max = 1000nS for standard mode
    
    I2C1->CR1 |= I2C_CR1_PE;                //enable i2c1
}



void i2c_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) = 0;
}



void i2c_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) = 1;
}



uint8_t i2c_poll(uint8_t i2c_addr)
{
    uint16_t SR1_tmp;
    uint16_t SR2_tmp;
    uint8_t attempts = 10;

    while (attempts)
    {
        //Start
        I2C1->CR1 |= I2C_CR1_START;
        //Wait for start generated
        while (!(I2C1->SR1 & I2C_SR1_SB))
        {
        }
        //Clear
        SR1_tmp = I2C1->SR1;

        //Device address
        I2C1->DR = i2c_addr;
        //Wait for address end of transmission or NAK reception
        do
        {
            SR1_tmp = I2C1->SR1;
        }
        while (!(SR1_tmp & I2C_SR1_ADDR) && !(SR1_tmp & I2C_SR1_AF));    //"ADDR is not set after a NACK reception"
        //Clear
        SR1_tmp = I2C1->SR1;
        SR2_tmp = I2C1->SR2;

        //Check acknowledge failure
        if (SR1_tmp & I2C_SR1_AF)
        {
            //no acknowledge is returned, slave is busy
            I2C1->SR1 = ~(I2C_SR1_AF);   //write 0 to clear AF bit

            attempts--;   //slave is not ready, decrement attmepts counter
        }
        else if (SR1_tmp & I2C_SR1_ADDR)
        {
            //Stop
            I2C1->CR1 |= I2C_CR1_STOP;
            while (I2C1->CR1 & I2C_CR1_STOP){} 		//wait for stop cleared by hardware

            return 1;   //slave is ready
        }
        else
        {
            attempts--; //???
        }
    }

    SR2_tmp = SR2_tmp + 1;

    //Stop before end
    I2C1->CR1 |= I2C_CR1_STOP;
    while (I2C1->CR1 & I2C_CR1_STOP){} 		//wait for stop cleared by hardware

    return 0;       //end of attmepts, slave is busy or absent
}
