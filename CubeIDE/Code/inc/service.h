/*
    SPOKE
    
    file: service.h
*/



#ifndef SERVICE_HEADER
#define SERVICE_HEADER



void delay_cyc(uint32_t cycles);
void print_debug(char *string);
void manage_power(void);
void call_bootloader(void);

uint32_t absv(int32_t value);
int32_t maxv(int32_t x, int32_t y);
int32_t minv(int32_t x, int32_t y);
int32_t limit_to(int32_t value, int32_t pos_lim, int32_t neg_lim);
void convert_timeout(uint32_t timeout_val, char *buffer);
void convert_main_distance(uint32_t distance, char *buffer);
void convert_main_alt_difference(int16_t dalt, char *buffer);
char *convert_heading(uint16_t heading);
uint8_t base64_encode(uint8_t *input, uint8_t *output, uint8_t length);
float atof32(char *input);
void ftoa32(float value, uint8_t precision, char *buffer);
int32_t atoi32(char *input);
void itoa32(int32_t value, char *buffer);
uint8_t string_length(char *str);
void string_copy(char *destination, char *source);
void string_cat(char *destination, char *source);
void byte2hex(uint8_t byte, char *array);
void time_date_add_leading_zero(char *buf);
uint8_t get_days_in_month(uint8_t month, uint8_t year);



#endif /*SERVICE_HEADER*/
