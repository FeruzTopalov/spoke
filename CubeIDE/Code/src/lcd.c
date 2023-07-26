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



//#define LCD_SSD1306		//todo: delete 1306
#define LCD_SH1106



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



uint8_t screen_buf[LCD_SIZE_BYTES];     		//public array 128x64 pixels
uint16_t buf_pos = 0;                   		//public var 0 - 1023
uint8_t current_page = 0;
uint8_t lcd_busy = 0;	//is lcd update ongoing?
uint8_t lcd_pending_off = 0; 	//pending command to off the lcd

uint8_t display_status = LCD_DISPLAY_ON;	//lcd panel status on/off



//SSD1306 init sequence (first byte in line = amount of config bytes in line)
const uint8_t ssd1306_conf[] =
{// len,  val1, val2, ...
    0x02, 0x20, 0x00,           /* horizontal adressing */ \
    0x02, 0xA1, 0xC8,           /* invert orientation */ \
	0x02, 0x81, 0x80,			/* set contrast */ \
    0x02, 0x8D, 0x14,		    /* enable charge pump */ \
	0x01, 0xAF,     			/* enable display */ \
    0x00                        /* end of the sequence */
};



//SH1106 init sequence (first byte in line = amount of config bytes in line)
const uint8_t sh1106_conf[] =
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



//LCD Init
void lcd_init(void)
{
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

#ifdef LCD_SSD1306
    while (ssd1306_conf[i] != 0x00)
    {
        len = ssd1306_conf[i++];
        
        cs_lcd_active();
        while (len--)
        {
            spi2_trx(ssd1306_conf[i++]);
        }
        cs_lcd_inactive();
    }
#endif

#ifdef LCD_SH1106
    while (sh1106_conf[i] != 0x00)
    {
        len = sh1106_conf[i++];

        cs_lcd_active();
        while (len--)
        {
            spi2_trx(sh1106_conf[i++]);
        }
        cs_lcd_inactive();
    }
#endif
    
    //spi2_clock_disable();

    lcd_clear();
    lcd_update();

}



void lcd_send_command(uint8_t command)
{
	//spi2_clock_enable();
	lcd_command_mode();
	cs_lcd_active();
	spi2_trx(command);
	cs_lcd_inactive();
	//spi2_clock_disable();
}



void lcd_display_on(void)
{
	lcd_send_command(LCD_COMMAND_DISPLAY_ON);
	display_status = LCD_DISPLAY_ON;
}



void lcd_display_off(void)
{
	lcd_send_command(LCD_COMMAND_DISPLAY_OFF);
	display_status = LCD_DISPLAY_OFF;
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



//Update screen with buffer content
void lcd_update(void)
{
	if (display_status == LCD_DISPLAY_ON) //update display content only if the display is on
	{
		//spi2_clock_enable();

#ifdef LCD_SSD1306
		lcd_data_mode();
		cs_lcd_active();
		for (uint16_t i = 0; i < LCD_SIZE_BYTES; i++)
		{
			spi2_trx(screen_buf[i]);
		}
		cs_lcd_inactive();
#endif

#ifdef LCD_SH1106
//		for (uint8_t page = 0; page < 8; page++)		//page means line for SH1106
//		{
//			lcd_send_command(0x02); 		//reset column address low to 2 because LCD panel is centered to SH1106 frame buffer
//			lcd_send_command(0x10);			//reset column address high
//			lcd_send_command(0xB0 | page);	//set page address
//
//			lcd_data_mode();
//			cs_lcd_active();
//			for (uint16_t i = (page * LCD_SIZE_X); i < ((page + 1) * LCD_SIZE_X); i++)
//			{
//				spi2_trx(screen_buf[i]);
//			}
//			cs_lcd_inactive();
//		}


		//todo: if current_page <> 0 to prevent glitches
		lcd_busy = 1;
		current_page = 0;
		lcd_send_command(0x02); 		//reset column address low to 2 because LCD panel is centered to SH1106 frame buffer
		lcd_send_command(0x10);			//reset column address high
		lcd_send_command(0xB0);			//set 0 page address
		lcd_data_mode();
		cs_lcd_active();
		spi2_dma_start(&screen_buf[0], LCD_SIZE_X);


#endif

		//spi2_clock_disable();
	}
}



void lcd_continue_update(void)
{
	current_page++;

	if (current_page < 8)
	{
		lcd_send_command(0x02); 		//reset column address low to 2 because LCD panel is centered to SH1106 frame buffer
		lcd_send_command(0x10);			//reset column address high
		lcd_send_command(0xB0 | current_page);	//set page address

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
            screen_buf[x + (y / 8) * LCD_SIZE_X] &= ~(1 << (y % 8));		////todo: use bit band instead
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
    screen_buf[x + (y / 8) * LCD_SIZE_X] |= 1 << (y % 8);			//todo: use bit band instead
}



//Set any pixel on the screen for XY plot (x 0-127, y 0-63)
//(0, 0) is bottom left corner
void lcd_set_pixel_plot(uint8_t x, uint8_t y)
{
    //set pixel
	y = 63 - y;
    screen_buf[x + (y / 8) * LCD_SIZE_X] |= 1 << (y % 8);			//todo: use bit band instead
}



//Set character position on screen (rows 0-3, cols 0-15)
void lcd_pos(uint8_t row, uint8_t col)
{
    buf_pos = (col + LCD_COLS * row * FONT_BYTES_Y) * FONT_BYTES_X;
}



//Put one char in buffer in position, defined previously via ssd1306_pos()
void lcd_char(char chr, uint8_t inv)
{
	uint16_t buf_pos_copy = buf_pos;
	uint16_t font_char_pos, c;
	uint8_t px;

    if (inv)
    {
        inv = 0xFF;
    }
    else
    {
        inv = 0x00;
    }

    //font_char_pos = ((uint8_t)chr - 32) * FONT_BYTES;
    font_char_pos = ((uint8_t)chr) * FONT_BYTES;

    for (px = 0, c = font_char_pos; px < FONT_BYTES_X; px++, c += 2)		//upper symbol row
    {
        screen_buf[buf_pos++] = font_8x16[c] ^ inv;
    }

    buf_pos = buf_pos_copy + LCD_SIZE_X; //point to the lower symbol's row
    for (px = 0, c = font_char_pos + 1; px < FONT_BYTES_X; px++, c += 2)	//lower symbol row
    {
        screen_buf[buf_pos++] = font_8x16[c] ^ inv;
    }

    buf_pos = buf_pos_copy + FONT_BYTES_X;	//point to the next LCD char
    //no new line/carriage return
}



void lcd_char16(char chr, uint8_t inv)
{
	uint16_t buf_pos_copy = buf_pos;
	uint16_t font_char_pos, c;
	uint8_t px;

    if (inv)			//todo: delete inv everywhere, it is useless
    {
        inv = 0xFF;
    }
    else
    {
        inv = 0x00;
    }

    font_char_pos = ((uint8_t)chr - 48) * FONT16_BYTES;

    for (px = 0, c = font_char_pos; px < FONT16_BYTES_X; px++, c += 2)		//upper symbol row
    {
        screen_buf[buf_pos++] = font_16x16[c] ^ inv;
    }

    buf_pos = buf_pos_copy + LCD_SIZE_X; //point to the lower symbol's row
    for (px = 0, c = font_char_pos + 1; px < FONT16_BYTES_X; px++, c += 2)	//lower symbol row
    {
        screen_buf[buf_pos++] = font_16x16[c] ^ inv;
    }

    buf_pos = buf_pos_copy + FONT16_BYTES_X;	//point to the next LCD char
    //no new line/carriage return
}



//Put one char in defined pos
void lcd_char_pos(uint8_t row, uint8_t col, char chr, uint8_t inv)
{
    lcd_pos(row, col);
    lcd_char(chr, inv);
}



//Put one char in defined pos
void lcd_char16_pos(uint8_t row, uint8_t col, char chr, uint8_t inv)
{
    lcd_pos(row, col);
    lcd_char16(chr, inv);
}



//Print string on screen (with position (rows 0-3, cols 0-15))
void lcd_print(uint8_t row, uint8_t col, char *p_str, uint8_t inv)
{
    lcd_pos(row, col);
    
    while (*p_str)
    {
        lcd_char(*p_str++, inv);
    }
}



//Clear, print, update
void lcd_print_only(uint8_t row, uint8_t col, char *p_str, uint8_t inv)
{
	lcd_clear();
	lcd_print(row, col, p_str, inv);
	lcd_update();
}




void lcd_print16(uint8_t row, uint8_t col, char *p_str, uint8_t inv)
{
    lcd_pos(row, col);

    while (*p_str)
    {
        lcd_char16(*p_str++, inv);
    }
}



//Print string on screen (with position) in viceversa direction (decrease collumn)
void lcd_print_viceversa(uint8_t row, uint8_t col, char *p_str, uint8_t inv)
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
        lcd_char(*--p_str, inv);
        buf_pos -= 2 * FONT_SIZE_X;         //minus two characters position
    }
}



//Print string on screen (with position) in viceversa direction (decrease collumn)
void lcd_print16_viceversa(uint8_t row, uint8_t col, char *p_str, uint8_t inv)
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
        lcd_char16(*--p_str, inv);
        buf_pos -= 4 * FONT_SIZE_X;         //minus two characters position
    }
}



//Print string on screen
void lcd_print_next(char *p_str, uint8_t inv)
{
    while (*p_str)
    {
        lcd_char(*p_str++, inv);
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



void lcd_draw_line(int8_t x1, int8_t y1, int8_t x2, int8_t y2)	//todo: implement new line draw algorithm to speed up this
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



uint8_t *get_lcd_buffer(void)
{
	return &screen_buf[0];
}
