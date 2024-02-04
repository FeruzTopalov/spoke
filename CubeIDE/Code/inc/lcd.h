/*
    SPOKE
    
    file: lcd.h
*/



#ifndef LCD_HEADER
#define LCD_HEADER



#define LCD_DISPLAY_ON	(1)
#define LCD_DISPLAY_OFF	(0)



#define LCD_COLS     		(16)
#define LCD_ROWS       		(4)
#define LCD_LAST_COL     	(LCD_COLS - 1)
#define LCD_LAST_ROW       	(LCD_ROWS - 1)



#define SYMB16_HOME			('=')
#define SYMB16_TIMEOUT      (':')
#define SYMB16_ALARM        (';')
#define SYMB16_FENCE        ('<')

#define SYMB8_DEGREE        ('\x80')
#define SYMB8_BAT_TAIL    	('\x81')
#define SYMB8_BAT_MID    	('\x82')
#define SYMB8_BAT_HEAD    	('\x83')
#define SYMB8_RX12        	('\x84')		//todo delete, not used
#define SYMB8_RX22        	('\x85')		//todo delete, not used
#define SYMB8_TIMEOUT       ('\x86')
#define SYMB8_FENCE       	('\x87')
#define SYMB8_ALARM       	('\x88')



#define LCD_SIZE_BYTES    	(1024)
#define LCD_SIZE_X        	(128)
#define LCD_SIZE_Y        	(64)



void lcd_init(void);
void lcd_send_command(uint8_t command);
void lcd_display_on(void);
void lcd_display_off(void);
void lcd_display_off_request(void);
uint8_t lcd_get_display_status(void);
void lcd_fill(void);
void lcd_clear(void);
void lcd_pixel(uint8_t x, uint8_t y, int8_t action);
void lcd_set_pixel(uint8_t x, uint8_t y);
void lcd_set_pixel_plot(uint8_t x, uint8_t y);
void lcd_pos(uint8_t row, uint8_t col);
void lcd_char(char chr);
void lcd_char16(char chr);
void lcd_char_pos(uint8_t row, uint8_t col, char chr);
void lcd_char16_pos(uint8_t row, uint8_t col, char chr);
void lcd_print(uint8_t row, uint8_t col, char *p_str);
void lcd_print_only(uint8_t row, uint8_t col, char *p_str);
void lcd_print16(uint8_t row, uint8_t col, char *p_str);
void lcd_print_next(char *p_str);
void lcd_print_viceversa(uint8_t row, uint8_t col, char *p_str);
void lcd_print_next_viceversa(char *p_str);
void lcd_print16_viceversa(uint8_t row, uint8_t col, char *p_str);
void lcd_bitmap(const uint8_t arr[]);
void lcd_update(void);
void lcd_continue_update(void);
void lcd_print_byte(uint8_t row, uint8_t col, uint8_t byte);
void lcd_byte2buf(uint16_t pos, uint8_t byte);
void lcd_draw_line(int8_t x1, int8_t y1, int8_t x2, int8_t y2);
void lcd_draw_dot(int8_t x0, int8_t y0);
uint8_t *get_lcd_buffer(void);
uint8_t get_lcd_busy(void);



#endif /*LCD_HEADER*/
