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

    i2c_clock_disable();
}



void i2c_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) = 0;
}



void i2c_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) = 1;
	delay_cyc(10);
}



uint8_t i2c_poll(uint8_t i2c_addr)
{
    uint16_t SR1_tmp;
    uint16_t SR2_tmp;
    uint8_t attempts = 10;

    i2c_clock_enable();

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

            i2c_clock_disable();

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

    i2c_clock_disable();

    return 0;       //end of attmepts, slave is busy or absent
}



void i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data)
{
	uint8_t SR_tmp;

	i2c_clock_enable();

	//Start
	I2C1->CR1 |= I2C_CR1_START;
	//Wait for start generated
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;

	//Device address
	I2C1->DR = i2c_addr;
	//Wait for address end of transmission
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;
	SR_tmp = I2C1->SR2;

	//Reg address
	I2C1->DR = reg_addr;
	//Wait for data register empty
	while (!(I2C1->SR1 & I2C_SR1_TXE))
	{
	}

	//Write data
	I2C1->DR = data;
	//Wait byte transfer finish
	while (!(I2C1->SR1 & I2C_SR1_BTF))
	{
	}

	SR_tmp++;

	//Stop
	I2C1->CR1 |= I2C_CR1_STOP;
	while (I2C1->CR1 & I2C_CR1_STOP){} 		//wait for stop cleared by hardware

	i2c_clock_disable();
}



uint8_t i2c_read(uint8_t i2c_addr, uint8_t reg_addr)
{
    uint8_t result = 0;
	uint16_t SR_tmp;

	i2c_clock_enable();

	//Start
	I2C1->CR1 |= I2C_CR1_START;
	//Wait for start generated
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;

	//Device address
	I2C1->DR = i2c_addr;
	//Wait for address end of transmission
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;
	SR_tmp = I2C1->SR2;

	//Reg address
	I2C1->DR = reg_addr;
	//Wait for data register empty
	while (!(I2C1->SR1 & I2C_SR1_TXE))
	{
	}


	//Start (restart actually)
	I2C1->CR1 |= I2C_CR1_START;
	//Wait for start generated
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;

	//Device address
	I2C1->DR = (i2c_addr | 0x01);	//read mode bit set
	//Wait for address end of transmission
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}

	//NACK next byte
	I2C1->CR1 &= ~I2C_CR1_ACK;

	//Clear ADR
	SR_tmp = I2C1->SR1;
	SR_tmp = I2C1->SR2;

	//Stop
	I2C1->CR1 |= I2C_CR1_STOP;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE))
	{
	}

	SR_tmp = SR_tmp + 1;

	//Read requested byte
	result = I2C1->DR;

	while (I2C1->CR1 & I2C_CR1_STOP){} 		//wait for stop cleared by hardware

	i2c_clock_disable();

    return result;
}



void i2c_read_multiple(uint8_t i2c_addr, uint8_t reg_addr, uint8_t size, uint8_t *buffer)
{
//	uint8_t remaining = size; //only for size > 2
	uint8_t SR_tmp;

	i2c_clock_enable();

	//Start
	I2C1->CR1 |= I2C_CR1_START;
	//Wait for start generated
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;

	//Device address
	I2C1->DR = i2c_addr;
	//Wait for address end of transmission
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;
	SR_tmp = I2C1->SR2;

	//Reg address
	I2C1->DR = (reg_addr | 0x80);	//to read multiple bytes of lsm303dlhc
	//Wait for data register empty
	while (!(I2C1->SR1 & I2C_SR1_TXE))
	{
	}


	//Start (restart actually)
	I2C1->CR1 |= I2C_CR1_START;
	//Wait for start generated
	while (!(I2C1->SR1 & I2C_SR1_SB))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;

	//Device address
	I2C1->DR = (i2c_addr | 0x01);	//read mode bit set
	//Wait for address end of transmission
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
	{
	}
	//Clear
	SR_tmp = I2C1->SR1;
	SR_tmp = I2C1->SR2;

//todo: wrap in cycle
	//ACK byte
	I2C1->CR1 |= I2C_CR1_ACK;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}

	//Read byte 1
	buffer[0] = I2C1->DR;



	//ACK byte
	I2C1->CR1 |= I2C_CR1_ACK;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}

	//Read byte 2
	buffer[1] = I2C1->DR;



	//ACK byte
	I2C1->CR1 |= I2C_CR1_ACK;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}

	//Read byte 3
	buffer[2] = I2C1->DR;



	//ACK byte
	I2C1->CR1 |= I2C_CR1_ACK;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}


/*
	//ACK byte
	I2C1->CR1 |= I2C_CR1_ACK;

	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}
*/

	//Wait byte transfer finish
	while (!(I2C1->SR1 & I2C_SR1_BTF))
	{
	}


	//NACK
	I2C1->CR1 &= ~I2C_CR1_ACK;


	//Read byte 4
	buffer[3] = I2C1->DR;


	//Stop
	I2C1->CR1 |= I2C_CR1_STOP;


	//Read byte 5
	buffer[4] = I2C1->DR;



	//Wait for data register not empty
	while (!(I2C1->SR1 & I2C_SR1_RXNE)){}

	//Read byte 6
	buffer[5] = I2C1->DR;

	while (I2C1->CR1 & I2C_CR1_STOP){} 		//wait for stop cleared by hardware

	SR_tmp = SR_tmp + 1;

	i2c_clock_disable();
}
