/*
    SPOKE
    
    file: service.c
*/

#include <string.h>
#include "stm32f10x.h"
#include "service.h"



//Simple delay in cycles
void delay_cyc(uint32_t cycles)
{
    while (cycles)
    {
        cycles--;
    }
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
