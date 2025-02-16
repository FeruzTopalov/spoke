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
uint32_t dec_pow_table[] = {1, 10, 100, 1000, 10000, 100000, 1000000};	//max precision 6 digits after point


uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 	//in non-leap year


const char hex_chars[] = "0123456789ABCDEF";


// Base64 character map
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


//Simple delay in cycles
void delay_cyc(uint32_t cycles)
{
    while (cycles--)
    {
    	__NOP();
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
    if ((RCC->CSR & RCC_CSR_SFTRSTF) || (RCC->CSR & RCC_CSR_IWDGRSTF))		//if the reset is caused by software (save & restart after settings changed) or by IWDG watchdog
    {
    	hold_power();
    }
    else
    {
    	release_power();					//initially set switch off position; hold off power when powering on

    	led_red_on();
    	led_green_on();

    	delay_cyc(600000); //startup delay ~2sec

    	led_red_off();
    	led_green_off();

    	hold_power();
    }
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

    //PB8 - Red LED
    GPIOB->CRH &= ~GPIO_CRH_MODE8_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE8_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF8;       //output push-pull

    //Port A
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    //PA15 - Power Switch Hold
    GPIOA->CRH &= ~GPIO_CRH_MODE15_0;   //output 2 MHz
    GPIOA->CRH |= GPIO_CRH_MODE15_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF15;      //output push-pull

    hold_power();	//hold power to prevent power off after software reset if bootloader will not be started; next time power controll see in manage_power()

    if (((GPIOB->IDR) & GPIO_IDR_IDR3) && !((GPIOB->IDR) & GPIO_IDR_IDR4)) //OK pressed, btn ESC released
    {
    	release_power();	//fixes "power-on lock" side effect

    	delay_cyc(2000000); //startup delay ~2sec
    	led_red_on();

		__set_MSP(*(uint32_t *)BootAddrF10xx);
		SysMemBootJump();
    }
}



uint32_t absv(int32_t value)
{
	if (value < 0)
	{
		return (-value);
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



    //convert
    day = timeout_val / (24 * 3600);
    timeout_val %= (24 * 3600);

    hour = timeout_val / 3600;
    timeout_val %= 3600;

    min = timeout_val / 60;
    sec = timeout_val % 60;



    //store print
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



uint8_t base64_encode(uint8_t *input, uint8_t *output, uint8_t length)
{
    //calculate Base64 output length
    uint8_t encoded_length = ((length + 2) / 3) * 4;

    //encode input data
    uint8_t in_idx = 0, out_idx = 0;
    while (in_idx + 3 <= length)
    {
    	output[out_idx++] = base64_chars[input[in_idx] >> 2];
    	output[out_idx++] = base64_chars[((input[in_idx] & 0x03) << 4) | (input[in_idx + 1] >> 4)];
    	output[out_idx++] = base64_chars[((input[in_idx + 1] & 0x0F) << 2) | (input[in_idx + 2] >> 6)];
    	output[out_idx++] = base64_chars[input[in_idx + 2] & 0x3F];
        in_idx += 3;
    }

    //handle remaining bytes (padding)
    if (in_idx < length)
    {
    	output[out_idx++] = base64_chars[input[in_idx] >> 2];
        if (in_idx + 1 < length)
        {
        	output[out_idx++] = base64_chars[((input[in_idx] & 0x03) << 4) | (input[in_idx + 1] >> 4)];
        	output[out_idx++] = base64_chars[(input[in_idx + 1] & 0x0F) << 2];
        }
        else
        {
        	output[out_idx++] = base64_chars[(input[in_idx] & 0x03) << 4];
        	output[out_idx++] = '=';
        }
        output[out_idx++] = '=';
    }

    return encoded_length;
}



//Converts string to float
float atof32(char *input)
{
    uint8_t i = 0;
    int32_t sign = 1;
    uint8_t p = 0;
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
        p++;
        i++;
    }

    return (sign * result / dec_pow_table[p]);
}



//Converts float to string
void ftoa32(float value, uint8_t precision, char *buffer)
{
    uint8_t i = 0;
    uint32_t mod = 0;
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

    value = value * dec_pow_table[precision];
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



uint8_t string_length(char *str)
{
	uint8_t length = 0;

    while (str[length] != '\0')
    {
        length++;
    }

    return length;
}



void byte2hex(uint8_t byte, char *array)
{
    array[0] = hex_chars[(byte >> 4) & 0x0F];
    array[1] = hex_chars[byte & 0x0F];
}



void time_date_add_leading_zero(char *buf)
{
    if (buf[1] == 0) //if single-char string, add leading zero
    {
    	buf[1] = buf[0];
    	buf[0] = '0';
    	buf[2] = 0;
    }
}



uint8_t get_days_in_month(uint8_t month, uint8_t year)	//month 1-12; year - last 2 digits
{
	uint16_t year_expanded = 2000 + year;		//in a hope that the first two year's digits are always "20"

	if (month == 2) //FEB
	{
		if ((year_expanded % 4 == 0 && year_expanded % 100 != 0) || (year_expanded % 400 == 0))
		{
			return 29; //FEB, leap year
		}
	}

	return days_in_month[month - 1];
}
