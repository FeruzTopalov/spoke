/*
    SPOKE
    
    file: service.c
*/

#include <string.h>
#include "stm32f10x.h"
#include "service.h"
#include "uart.h"
#include "gpio.h"



void (*SysMemBootJump)(void);



char rumbs[9][3] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"};



//Simple delay in cycles
void delay_cyc(uint32_t cycles)
{
    while (cycles)
    {
        cycles--;
    }
}



void print_debug(char *string)
{
    while (*string)
    {
    	uart1_tx_byte(*string++);
    }
}



void manage_power(void)
{
    release_power();					//initially set switch off position
    if (!(RCC->CSR & RCC_CSR_SFTRSTF))		//if the reset is not caused by software (save & restart after settings changed)
    {
    	delay_cyc(600000); //startup delay ~2sec
    }
	hold_power();
}



//This func is called once at power up and prior to any MCU configuration, see system_stm32f10x.c
//Startup condition: btn OK pressed, btn ESC released
void call_bootloader(void)
{
	uint32_t BootAddrF10xx  = 0x1FFFF000;
	SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddrF10xx + 4))));

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;         //enable afio clock
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    //Port B
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    //PB4 - Button OK
    GPIOB->CRL &= ~GPIO_CRL_MODE4;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF4_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF4_1;
    GPIOB->ODR |= GPIO_ODR_ODR4;        //pull-up on

    //PB3 - Button ESC
    GPIOB->CRL &= ~GPIO_CRL_MODE3;      //input mode
    GPIOB->CRL &= ~GPIO_CRL_CNF3_0;     //input with pull
    GPIOB->CRL |= GPIO_CRL_CNF3_1;
    GPIOB->ODR |= GPIO_ODR_ODR3;        //pull-up on

    if (((GPIOB->IDR) & GPIO_IDR_IDR3) && !((GPIOB->IDR) & GPIO_IDR_IDR4))
    {
		__set_MSP(*(uint32_t *)BootAddrF10xx);
		SysMemBootJump();
    }
}



uint32_t absv(int32_t value)
{
	if (value < 0)
	{
		return (-1 * value);
	}
	else
	{
		return value;
	}
}



int32_t maxv(int32_t x, int32_t y)
{
	if (x > y)
	{
		return x;
	}
	else
	{
		return y;
	}
}



int32_t minv(int32_t x, int32_t y)
{
	if (x < y)
	{
		return x;
	}
	else
	{
		return y;
	}
}



int32_t limit_to(int32_t value, int32_t pos_lim, int32_t neg_lim)
{
	if (value > pos_lim)
	{
		return pos_lim;
	}
	else if (value < neg_lim)
	{
		return neg_lim;
	}
	else
	{
		return value;
	}
}



void convert_timeout(uint32_t timeout_val, char *buffer)
{
    uint32_t sec = 0;
    uint32_t min = 0;
    uint32_t hour = 0;
    uint32_t day = 0;
    char buf[3];


    if (timeout_val >= 60)
    {
        min = timeout_val / 60;
        sec = timeout_val % 60;

        if (min >= 60)
        {
            hour = min / 60;
            min = min % 60;

            if (hour >= 24)
            {
                day = hour / 24;
                hour = hour % 24;
            }
        }
    }
    else
    {
        sec = timeout_val;
    }

    if (day)
    {
        //XXdXXh
        itoa32(day, &buf[0]);
        if (day > 9)
        {
            buffer[0] = buf[0];
            buffer[1] = buf[1];
            buffer[2] = 'd';
        }
        else
        {
            buffer[0] = '0';
            buffer[1] = buf[0];
            buffer[2] = 'd';
        }

        itoa32(hour, &buf[0]);
        if (hour > 9)
        {
            buffer[3] = buf[0];
            buffer[4] = buf[1];
            buffer[5] = 'h';
        }
        else
        {
            buffer[3] = '0';
            buffer[4] = buf[0];
            buffer[5] = 'h';
        }
    }
    else if (hour)
    {
        //XXhXXm
        itoa32(hour, &buf[0]);
        if (hour > 9)
        {
            buffer[0] = buf[0];
            buffer[1] = buf[1];
            buffer[2] = 'h';
        }
        else
        {
            buffer[0] = '0';
            buffer[1] = buf[0];
            buffer[2] = 'h';
        }

        itoa32(min, &buf[0]);
        if (min > 9)
        {
            buffer[3] = buf[0];
            buffer[4] = buf[1];
            buffer[5] = 'm';
        }
        else
        {
            buffer[3] = '0';
            buffer[4] = buf[0];
            buffer[5] = 'm';
        }
    }
    else
    {
        //XXmXXs
        itoa32(min, &buf[0]);
        if (min > 9)
        {
            buffer[0] = buf[0];
            buffer[1] = buf[1];
            buffer[2] = 'm';
        }
        else
        {
            buffer[0] = '0';
            buffer[1] = buf[0];
            buffer[2] = 'm';
        }

        itoa32(sec, &buf[0]);
        if (sec > 9)
        {
            buffer[3] = buf[0];
            buffer[4] = buf[1];
            buffer[5] = 's';
        }
        else
        {
            buffer[3] = '0';
            buffer[4] = buf[0];
            buffer[5] = 's';
        }
    }

    buffer[6] = 0;  //string end
}



void convert_main_distance(uint32_t distance, char *buffer)
{
	float tmpf = 0;

	if (distance < 10000)
	{
		itoa32(distance, &buffer[0]);
	}
	else if (distance < 100000)
	{
		tmpf = distance / 1000.0;
		ftoa32(tmpf, 1, &buffer[0]);
	}
	else if (distance < 1000000)
	{
		distance = distance / 1000;
		itoa32(distance, &buffer[0]);
		buffer[3] = '.';	//append '.'
		buffer[4] = 0;
	}
	else
	{
		buffer[0] = 'X';
		buffer[1] = 0;
	}
}



void convert_main_alt_difference(int16_t dalt, char *buffer)
{
	float tmpf = 0;

	if (absv(dalt) < 10000)
	{
		itoa32(dalt, &buffer[0]);
	}
	else if (absv(dalt) < 100000)
	{
		tmpf = dalt / 1000.0;
		ftoa32(tmpf, 1, &buffer[0]);
	}
	else
	{
		buffer[0] = 'X';
		buffer[1] = 0;
	}
}



char *convert_heading(uint16_t heading)
{
	float rumb;

	rumb = (heading + 22.5) / 45;
	return &rumbs[(uint8_t)rumb][0];
}



//Converts string to float
float atof32(char *input)
{
    uint8_t i = 0;
    int32_t sign = 1;
    float power = 1.0;
    float result = 0.0;

    if(input[0] == 0)
    {
        return 0.0;
    }

    if(input[i] == '-')
    {
        sign = -1;
        i++;
    }

    while(input[i] != '.')
    {
        result = result * 10.0 + (input[i] - '0');
        i++;
    }

    i++;

    while(input[i] != 0)
    {
        result = result * 10.0 + (input[i] - '0');
        power *= 10.0;
        i++;
    }

    return (sign * result / power);
}



//Converts float to string
void ftoa32(float value, uint8_t precision, char *buffer)
{
    uint8_t i = 0;
    uint32_t mod = 0;
    float pow = 1.0;
    char sgn = 0;
    float value_copy;

    if((value == 0.0) || (value == -0.0))
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }

    if(value < 0)
    {
        sgn = '-';
        value *= -1.0;
    }

    value_copy = value;

    for(uint8_t p = 0; p < precision; p++)
    {
        pow = pow * 10.0;
    }

    value = value * pow;
    uint32_t ipart = value;

    buffer[i++] = 0;

    do
    {
        mod = ipart % 10;
        ipart /= 10;
        buffer[i++] = mod + '0';
    }
    while(ipart > 0);

    if (value_copy < 1.0)
    {
        for(uint8_t p = 0; p < precision; p++)
        {
            buffer[i] = '0';
            i++;
        }
    }

    if(sgn == '-')
    {
        buffer[i] = sgn;
    }
    else
    {
        i--;
    }

    for(uint8_t n = i + 1; n > precision + 1; n--)
    {
        buffer[n] = buffer[n - 1];
    }
    buffer[precision + 1] = '.';
    i++;

    char c;
    for(uint8_t j = 0; j < i; j++, i--)
    {
        c = buffer[j];
        buffer[j] = buffer[i];
        buffer[i] = c;
    }
}



//Converts string to integer
int32_t atoi32(char *input)
{
    uint8_t i = 0;
    int32_t sign = 1;
    int32_t result = 0;

    if(input[0] == 0)
    {
        return 0;
    }

    if((input[0] == '0') && (input[1] == 0))
    {
        return 0;
    }

    if(input[i] == '-')
    {
        sign = -1;
        i++;
    }

    while(input[i] != 0)
    {
        result = result * 10 + (input[i] - '0');
        i++;
    }

    return result * sign;
}



//Converts integer to string
void itoa32(int32_t value, char *buffer)
{
    uint8_t i = 0;
    uint8_t mod = 0;
    char sgn = 0;

    if(value == 0)
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }

    if(value < 0)
    {
        sgn = '-';
        value *= -1;
    }

    buffer[i++] = 0;

    while(value > 0)
    {
        mod = value % 10;
        value /= 10;
        buffer[i++] = mod + '0';
    }

    if(sgn == '-')
    {
        buffer[i] = sgn;
    }
    else
    {
        i--;
    }

    char c;
    for(uint8_t j = 0; j < i; j++, i--)
    {
        c = buffer[j];
        buffer[j] = buffer[i];
        buffer[i] = c;
    }
}



void add_leading_zero(char *buf)	//todo: move in c file where it is used
{
    if (buf[1] == 0)
    {
    	buf[1] = buf[0];
    	buf[0] = '0';
    	buf[2] = 0;
    }
}
