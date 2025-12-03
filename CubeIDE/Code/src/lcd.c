/*
    SPOKE
    
    file: lcd.c
*/

#include <string.h>
#include <math.h>
#include "stm32f10x.h"
#include "lcd_font8x16.h"
#include "lcd_font16x16.h"
#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "service.h"
#include "config.h"
#include "timer.h"
#include "settings.h"



void lcd_backlight_enable(void);
void lcd_backlight_disable(void);



#define LCD_SIZE_BYTES    	(1024)


#define FONT_SIZE_X       	(8)		//size of font in pixels
#define FONT_SIZE_Y       	(16)	//size of font in pixels
#define FONT_BYTES_X      	(8)		//size of font in bytes
#define FONT_BYTES_Y      	(2)		//size of font in bytes
#define FONT_BYTES      	(FONT_BYTES_X * FONT_BYTES_Y)	//size of font in bytes

#define FONT16_SIZE_X       	(16)	//size of font in pixels
#define FONT16_SIZE_Y       	(16)	//size of font in pixels
#define FONT16_BYTES_X      	(16)	//size of font in bytes
#define FONT16_BYTES_Y      	(2)		//size of font in bytes
#define FONT16_BYTES      		(FONT16_BYTES_X * FONT16_BYTES_Y)	//size of font in bytes

#define LCD_COMMAND_DISPLAY_ON	(0xAF)
#define LCD_COMMAND_DISPLAY_OFF	(0xAE)
#define LCD_COMMAND_SET_ROW_ADR_BASE	(0xB0)
#define LCD_COMMAND_SET_COL_ADRH_BASE	(0x10)

#ifdef LCD_TYPE_SH1106
#define LCD_COMMAND_SET_COL_ADRL_BASE	(0x02)		//LCD panel is centered to SH1106 frame buffer
#endif

#ifdef LCD_TYPE_ST7567A
#define LCD_COMMAND_SET_COL_ADRL_BASE	(0x00)		//LCD panel is left-adjusted to ST7567A frame buffer
#endif

uint8_t screen_buf[LCD_SIZE_BYTES];     		//public array 128x64 pixels
uint16_t buf_pos = 0;                   		//public var 0 - 1023
uint8_t current_page = 0;
uint8_t lcd_busy = 0;	//is lcd update ongoing?
uint8_t lcd_pending_off = 0; 	//pending command to off the lcd

uint8_t display_status = LCD_DISPLAY_ON;	//lcd panel status on/off
uint8_t current_backlight_option = BL_LEVEL_OFF_SETTING; //latest (current) backlight option

struct settings_struct *p_settings;


#ifdef LCD_TYPE_SH1106
//SH1106 init sequence (first byte in line = amount of config bytes in line)
const uint8_t lcd_conf[] =
{// len,  val1, val2, ...
    0x01, 0x00,			/* Set lower column address 0 */ \
    0x01, 0x10,			/* Set higher column address 0 */ \
	0x01, 0x40,			/* Set display start line 0 */ \
	0x01, 0xB0,			/* Set page address 0 */ \
	0x01, 0xA1,			/* Invert orientation */ \
	0x01, 0xC8,			/* Reverse scan direction */ \
	0x01, 0xAF,			/* enable display */ \
    0x00				/* end of the sequence */
};
#endif



#ifdef LCD_TYPE_ST7567A
//ST7567A init sequence
const uint8_t lcd_conf[] =
{// len,  val1, val2, ...
    0x01, 0xA2,			/* Set 1/9 bias */ \
    0x01, 0xA0,			/* Set SEG normal direction */ \
	0x01, 0xC8,			/* Set COM inverted direction */ \
	0x01, 0x24,			/* Set regulation ration 5.0 */ \
	0x02, 0x81,	0x28,	/* Set EV command, set EV = 40 */ \

						/* BEGIN - DO NOT SPLIT */ \
	0x01, 0xFF,			/* Enter Extension Command Set mode */ \
	0x01, 0x72,			/* Enter Display Setting mode */ \
	0x01, 0xFE,			/* Exit Extension Command Set mode */ \

	0x01, 0xD4,			/* Set Duty 1/65 */ \
	0x01, 0x90,			/* Set Bias 1/9 */ \
	0x01, 0x9B,			/* Set FPS 190 */ \

	0x01, 0xFF,			/* Enter Extension Command Set mode */ \
	0x01, 0x70,			/* Exit Display Setting mode */ \
	0x01, 0xFE,			/* Exit Extension Command Set mode */ \
						/* END - DO NOT SPLIT */ \

	0x01, 0x2C,			/* Booster On */ \
	0x01, 0x2E,			/* Regulator On */ \
	0x01, 0x2F,			/* Follower On */ \
    0x01, 0x00,			/* Set lower column address 0 */ \
    0x01, 0x10,			/* Set higher column address 0 */ \
	0x01, 0x40,			/* Set display start line 0 */ \
	0x01, 0xB0,			/* Set page address 0 */ \
	0x01, 0xAF,			/* enable display */ \
    0x00				/* end of the sequence */
};
#endif



//LCD Init
void lcd_init(void)
{
	p_settings = get_settings();
	current_backlight_option = p_settings->bl_level_opt;
	lcd_toggle_backlight_opt(current_backlight_option); //initialize backlight with saved setting

	spi2_clock_enable();

    cs_lcd_inactive();      //ports init state
    res_lcd_inactive();
    lcd_command_mode();
    delay_cyc(10000);
    
    res_lcd_active();       //reset lcd
    delay_cyc(10000);
    res_lcd_inactive();
    delay_cyc(10000);
    
    uint8_t i = 0;
    uint8_t len = 0;

    while (lcd_conf[i] != 0x00)
    {
        len = lcd_conf[i++];

        cs_lcd_active();
        while (len--)
        {
            spi2_tx(lcd_conf[i++]);
        }
        cs_lcd_inactive();
    }

    spi2_clock_disable();
}



void lcd_send_command(uint8_t command)
{
	spi2_clock_enable();
	lcd_command_mode();
	cs_lcd_active();
	spi2_tx(command);
	cs_lcd_inactive();
	spi2_clock_disable();
}



void lcd_display_on(void)
{
	lcd_send_command(LCD_COMMAND_DISPLAY_ON);
	display_status = LCD_DISPLAY_ON;

	lcd_toggle_backlight_opt(current_backlight_option); //fed it with own current_backlight_option state because upon LCD ON the current option could be LCD_COMMAND_DISPLAY_OFF
}



void lcd_display_off(void)
{
	lcd_send_command(LCD_COMMAND_DISPLAY_OFF);
	display_status = LCD_DISPLAY_OFF;

	lcd_backlight_disable();
}



void lcd_display_off_request(void)
{
	if (lcd_busy == 1)
	{
		lcd_pending_off = 1;
	}
	else
	{
		lcd_display_off();
	}
}



uint8_t lcd_get_display_status(void)
{
	return display_status;
}



void lcd_toggle_backlight_opt(uint8_t bl_option)
{
	current_backlight_option = bl_option;	//get value from menu and set it as current

	if (current_backlight_option == BL_LEVEL_OFF_SETTING)
	{
		lcd_backlight_disable();
	}
	else
	{
		lcd_backlight_enable();
	}
}



void lcd_backlight_enable(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT

	uint8_t bl_pwm_timer_level;

	switch (current_backlight_option)
	{
		case BL_LEVEL_LOW_AUTO_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_LOW;
			break;

		case BL_LEVEL_LOW_CONSTANT_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_LOW;
			break;

		case BL_LEVEL_MID_AUTO_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_MID;
			break;

		case BL_LEVEL_MID_CONSTANT_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_MID;
			break;

		case BL_LEVEL_HIGH_AUTO_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_MAX;
			break;

		case BL_LEVEL_HIGH_CONSTANT_SETTING:
			bl_pwm_timer_level = BL_PWM_LEVEL_MAX;
			break;

		default:
			bl_pwm_timer_level = BL_PWM_LEVEL_OFF;
			break;
	}

	backlight_pwm_set(bl_pwm_timer_level);

#else
	backlight_lcd_high();
#endif
}



void lcd_backlight_disable(void)
{
#ifdef	SPLIT_PWM_BUZZER_BACKLIGHT
	backlight_pwm_set(BL_PWM_LEVEL_OFF);
#else
	backlight_lcd_low();
#endif
}



//Update screen with buffer content
void lcd_update(void)
{
	if (!lcd_busy)
	{
		if (display_status == LCD_DISPLAY_ON) //update display content only if the display is on
		{
			lcd_busy = 1;
			current_page = 0;
			lcd_send_command(LCD_COMMAND_SET_COL_ADRL_BASE); 		//reset column address low
			lcd_send_command(LCD_COMMAND_SET_COL_ADRH_BASE);		//reset column address high
			lcd_send_command(LCD_COMMAND_SET_ROW_ADR_BASE);			//set 0 page address
			lcd_data_mode();
			cs_lcd_active();
			spi2_dma_start(&screen_buf[0], LCD_SIZE_X);
		}
	}
}



void lcd_continue_update(void)
{
	current_page++;

	if (current_page < 8)
	{
		lcd_send_command(LCD_COMMAND_SET_COL_ADRL_BASE); 		//reset column address low
		lcd_send_command(LCD_COMMAND_SET_COL_ADRH_BASE);			//reset column address high
		lcd_send_command(LCD_COMMAND_SET_ROW_ADR_BASE | current_page);	//set page address

		lcd_data_mode();
		cs_lcd_active();
		spi2_dma_start(&screen_buf[current_page * LCD_SIZE_X], LCD_SIZE_X);
	}
	else
	{
		current_page = 0;
		if (lcd_pending_off == 1)
		{
			lcd_display_off();
			lcd_pending_off = 0;
		}
		lcd_busy = 0;
	}
}



//Fill all screen buffer pixels
void lcd_fill(void)
{
    for (uint16_t i = 0; i < LCD_SIZE_BYTES; i++)
    {
        screen_buf[i] = 0xFF;
    }
}



//Clear all screen buffer pixels
void lcd_clear(void)
{
    memset(screen_buf, 0, LCD_SIZE_BYTES);
}



//Clear, Set or Reset any pixel on the screen (x 0-127, y 0-63)
//(0, 0) is top left corner
void lcd_pixel(uint8_t x, uint8_t y, int8_t action)
{
    switch (action)
    {
        case 0:     //clear pixel
            screen_buf[x + (y / 8) * LCD_SIZE_X] &= ~(1 << (y % 8));
            break;
        case 1:     //set pixel
            screen_buf[x + (y / 8) * LCD_SIZE_X] |= 1 << (y % 8);
            break;
        default:    //invert pixel
            screen_buf[x + (y / 8) * LCD_SIZE_X] ^= 1 << (y % 8);
            break;
    }
}



//Set any pixel on the screen (x 0-127, y 0-63)
//(0, 0) is top left corner
void lcd_set_pixel(uint8_t x, uint8_t y)
{
    //set pixel
    screen_buf[x + (y / 8) * LCD_SIZE_X] |= 1 << (y % 8);
}



//Set any pixel on the screen for XY plot (x 0-127, y 0-63)
//(0, 0) is bottom left corner
void lcd_set_pixel_plot(uint8_t x, uint8_t y)
{
    //set pixel
	y = 63 - y;
    screen_buf[x + (y / 8) * LCD_SIZE_X] |= 1 << (y % 8);
}



//Set character position on screen (rows 0-3, cols 0-15)
void lcd_pos(uint8_t row, uint8_t col)
{
    buf_pos = (col + LCD_COLS * row * FONT_BYTES_Y) * FONT_BYTES_X;
}



//Put one char in buffer in position, defined previously via ssd1306_pos()
void lcd_char(char chr)
{
	uint16_t buf_pos_copy = buf_pos;
	uint16_t font_char_pos, c;
	uint8_t px;

    font_char_pos = ((uint8_t)chr) * FONT_BYTES;

    for (px = 0, c = font_char_pos; px < FONT_BYTES_X; px++, c += 2)		//upper symbol row
    {
        screen_buf[buf_pos++] = font_8x16[c];
    }

    buf_pos = buf_pos_copy + LCD_SIZE_X; //point to the lower symbol's row
    for (px = 0, c = font_char_pos + 1; px < FONT_BYTES_X; px++, c += 2)	//lower symbol row
    {
        screen_buf[buf_pos++] = font_8x16[c];
    }

    buf_pos = buf_pos_copy + FONT_BYTES_X;	//point to the next LCD char
    //no new line/carriage return
}



//Put one char in buffer in position with content overlay, defined previously via ssd1306_pos()
void lcd_char_overlay(char chr)
{
	uint16_t buf_pos_copy = buf_pos;
	uint16_t font_char_pos, c;
	uint8_t px;

    font_char_pos = ((uint8_t)chr) * FONT_BYTES;

    for (px = 0, c = font_char_pos; px < FONT_BYTES_X; px++, c += 2)		//upper symbol row
    {
        screen_buf[buf_pos++] |= font_8x16[c];
    }

    buf_pos = buf_pos_copy + LCD_SIZE_X; //point to the lower symbol's row
    for (px = 0, c = font_char_pos + 1; px < FONT_BYTES_X; px++, c += 2)	//lower symbol row
    {
        screen_buf[buf_pos++] |= font_8x16[c];
    }

    buf_pos = buf_pos_copy + FONT_BYTES_X;	//point to the next LCD char
    //no new line/carriage return
}



void lcd_char16(char chr)
{
	uint16_t buf_pos_copy = buf_pos;
	uint16_t font_char_pos, c;
	uint8_t px;

    font_char_pos = ((uint8_t)chr - 48) * FONT16_BYTES;

    for (px = 0, c = font_char_pos; px < FONT16_BYTES_X; px++, c += 2)		//upper symbol row
    {
        screen_buf[buf_pos++] = font_16x16[c];
    }

    buf_pos = buf_pos_copy + LCD_SIZE_X; //point to the lower symbol's row
    for (px = 0, c = font_char_pos + 1; px < FONT16_BYTES_X; px++, c += 2)	//lower symbol row
    {
        screen_buf[buf_pos++] = font_16x16[c];
    }

    buf_pos = buf_pos_copy + FONT16_BYTES_X;	//point to the next LCD char
    //no new line/carriage return
}



//Put one char in defined pos
void lcd_char_pos(uint8_t row, uint8_t col, char chr)
{
    lcd_pos(row, col);
    lcd_char(chr);
}



//Put one char in defined pos, with overlay
void lcd_char_overlay_pos(uint8_t row, uint8_t col, char chr)
{
    lcd_pos(row, col);
    lcd_char_overlay(chr);
}



//Put one char in defined pos
void lcd_char16_pos(uint8_t row, uint8_t col, char chr)
{
    lcd_pos(row, col);
    lcd_char16(chr);
}



//Print string on screen (with position (rows 0-3, cols 0-15))
void lcd_print(uint8_t row, uint8_t col, char *p_str)
{
    lcd_pos(row, col);
    
    while (*p_str)
    {
        lcd_char(*p_str++);
    }
}



//Clear, print, update
void lcd_print_only(uint8_t row, uint8_t col, char *p_str)
{
	while (lcd_busy);		//wait until free
	lcd_clear();
	lcd_print(row, col, p_str);
	lcd_update();
}




void lcd_print16(uint8_t row, uint8_t col, char *p_str)
{
    lcd_pos(row, col);

    while (*p_str)
    {
        lcd_char16(*p_str++);
    }
}



//Print string on screen (with position) in viceversa direction (decrease collumn)
void lcd_print_viceversa(uint8_t row, uint8_t col, char *p_str)
{
    uint8_t symb_cntr = 0;
    
    lcd_pos(row, col);
    
    while (*p_str)
    {
        p_str++;
        symb_cntr++;
    }
    
    while (symb_cntr)
    {
        symb_cntr--;
        lcd_char(*--p_str);
        buf_pos -= 2 * FONT_SIZE_X;         //minus two characters position
    }
}



//Print string on screen (with position) in viceversa direction (decrease collumn)
void lcd_print16_viceversa(uint8_t row, uint8_t col, char *p_str)
{
    uint8_t symb_cntr = 0;

    lcd_pos(row, col);

    while (*p_str)
    {
        p_str++;
        symb_cntr++;
    }

    while (symb_cntr)
    {
        symb_cntr--;
        lcd_char16(*--p_str);
        buf_pos -= 4 * FONT_SIZE_X;         //minus two characters position
    }
}



//Print string on screen
void lcd_print_next(char *p_str)
{
    while (*p_str)
    {
        lcd_char(*p_str++);
    }
}



void lcd_print_next_viceversa(char *p_str)	//use only after lcd_print_viceversa()
{
    uint8_t symb_cntr = 0;

    //buf_pos -= 4 * FONT_SIZE_X;         //minus two characters position

    while (*p_str)
    {
        p_str++;
        symb_cntr++;
    }

    while (symb_cntr)
    {
        symb_cntr--;
        lcd_char(*--p_str);
        buf_pos -= 2 * FONT_SIZE_X;         //minus two characters position
    }
}



//Show bitmap
void lcd_bitmap(const uint8_t arr[])
{
    for (uint16_t i = 0; i < LCD_SIZE_BYTES; i++)
    {
        screen_buf[i] = arr[i];
    }
}



//Print byte on screen (debug function)
void lcd_print_byte(uint8_t row, uint8_t col, uint8_t byte)
{
    lcd_pos(row, col);
    screen_buf[buf_pos++] = byte;
}



void lcd_byte2buf(uint16_t pos, uint8_t byte)
{
    screen_buf[pos++] = byte;
}



void lcd_draw_line(int8_t x1, int8_t y1, int8_t x2, int8_t y2)
{
	int8_t signX;
	int8_t signY;
	int8_t deltaX;
	int8_t deltaY;

	if (x1 < x2)
	{
		signX = 1;
		deltaX = x2 - x1;
	}
	else
	{
		signX = -1;
		deltaX = x1 - x2;
	}


	if (y1 < y2)
	{
		signY = 1;
		deltaY = y2 - y1;
	}
	else
	{
		signY = -1;
		deltaY = y1 - y2;
	}


	int8_t error = deltaX - deltaY;
	int8_t error2;



	lcd_set_pixel(x2, y2);

    while((x1 != x2) || (y1 != y2))
    {

    	lcd_draw_dot(x1, y1);

        error2 = error * 2;
        if(error2 > -deltaY)
        {
          error -= deltaY;
          x1 += signX;
        }


        if(error2 < deltaX)
        {
          error += deltaX;
          y1 += signY;
        }

    }
}


void lcd_draw_dot(int8_t x0, int8_t y0)
{
	uint8_t loop = 0;

	//            draw a filled circle as filled rect but with skipped corners
	//             xxx
	//            xxxxx
	//            xxoxx
	//            xxxxx
	//             xxx

	for (int8_t px = -2; px <= 2; px++)
	{
		for (int8_t py = -2; py <= 2; py++)
		{
			loop++;

			if ((loop == 1) || (loop == 5) || (loop == 21) || (loop == 25))
			{
				continue;
			}
			else
			{
				lcd_set_pixel(x0 + px, y0 + py);
			}
		}
	}
}



void lcd_draw_big_dot(int8_t x0, int8_t y0)
{
	for (int8_t px = -3; px <= 3; px++)
	{
		for (int8_t py = -3; py <= 3; py++)
		{
			if ((absv(px) + absv(py)) < 5)
			{
				lcd_set_pixel(x0 + px, y0 + py);
			}
		}
	}
}



uint8_t *get_lcd_buffer(void)
{
	return &screen_buf[0];
}



uint8_t get_lcd_busy(void)
{
	return lcd_busy;
}
