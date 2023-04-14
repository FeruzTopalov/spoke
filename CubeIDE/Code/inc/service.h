/*
    SPOKE
    
    file: service.h
*/



#ifndef SERVICE_HEADER
#define SERVICE_HEADER



void delay_cyc(uint32_t cycles);

float atof32(char *input);
void ftoa32(float value, uint8_t precision, char *buffer);
int32_t atoi32(char *input);
void itoa32(int32_t value, char *buffer);
void add_leading_zero(char *buf);



#endif /*SERVICE_HEADER*/
