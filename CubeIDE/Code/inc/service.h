/*
    SPOKE
    
    file: service.h
*/



#ifndef SERVICE_HEADER
#define SERVICE_HEADER



void delay_cyc(uint32_t cycles);
void print_debug(char *string);
void manage_power(void);

uint32_t absv(int32_t value);
int32_t maxv(int32_t x, int32_t y);
int32_t minv(int32_t x, int32_t y);
int32_t limit_to(int32_t value, int32_t pos_lim, int32_t neg_lim);
void convert_timeout(uint32_t timeout_val, char *buffer);
float atof32(char *input);
void ftoa32(float value, uint8_t precision, char *buffer);
int32_t atoi32(char *input);
void itoa32(int32_t value, char *buffer);
void add_leading_zero(char *buf);



#endif /*SERVICE_HEADER*/
