/*
    SPOKE
    
    file: gpio.h
*/



#ifndef GPIO_HEADER
#define GPIO_HEADER



void gpio_init(void);
void ext_int_init(void);
void enable_buttons_interrupts(void);
void disable_buttons_interrupts(void);
void clear_buttons_interrupts(void);

void mux_console_usb(void);
void mux_console_ble(void);
void backlight_lcd_high(void);
void backlight_lcd_low(void);

void led_red_on(void);
void led_red_off(void);
void led_green_on(void);
void led_green_off(void);

void cs_rf_active(void);
void cs_rf_inactive(void);

void res_rf_active(void);
void res_rf_inactive(void);

void res_lcd_active(void);
void res_lcd_inactive(void);

void res_lcd_active(void);
void res_lcd_inactive(void);

void lcd_data_mode(void);
void lcd_command_mode(void);

void cs_lcd_active(void);
void cs_lcd_inactive(void);

void hold_power(void);
void release_power(void);

void rf_tx_mode(void);
void rf_rx_mode(void);

#endif /*GPIO_HEADER*/
