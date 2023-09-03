/*
    SPOKE
    
    file: menu.c
*/



#include <math.h>
#include <string.h>
#include "stm32f10x.h"
#include "config.h"
#include "menu.h"
#include "buttons.h"
#include "lcd.h"
#include "main.h"
#include "gpio.h"
#include "timer.h"
#include "settings.h"
#include "service.h"
#include "lrns.h"
#include "points.h"
#include "lcd_bitmaps.h"
#include "gps.h"
#include "compass.h"
#include "adc.h"



char *FW_VERSION = "1.0";	//firmware
char *HW_VERSION = "2.5";		//hardware (2.5 means new firmware on old HW v2)



uint8_t get_current_item(void);
uint8_t get_last_item(void);
void set_current_item(uint8_t new_value);
void reset_current_item_in_menu(uint8_t menu);



void scroll_up(void);
void scroll_down(void);
void switch_forward(void);
void switch_backward(void);



void draw_main(void);
void draw_bat_level(void);
void draw_devices(void);
void draw_power(void);
void draw_navigation(void);
void draw_coordinates(void);
void draw_device_submenu(void);
void draw_save_device(void);
void draw_delete_device(void);
void draw_saved_popup(void);
void draw_deleted_popup(void);
void draw_settings(void);
void draw_edit_device(void);
void draw_edit_radio(void);
void draw_edit_other(void);
void draw_set_device_number(void);
void draw_set_device_id(void);
void draw_set_freq_channel(void);
void draw_set_tx_power(void);
void draw_set_update_interval(void);
void draw_set_timeout(void);
void draw_set_fence(void);
void draw_set_timezone(void);
void draw_confirm_settings_save(void);



void navigation_ok(void);
void scroll_devices_up(void);
void navigation_and_coordinates_up(void);
void navigation_and_coordinates_down(void);
void navigation_esc(void);
void coordinates_esc(void);
void main_ok(void);
void main_navigation_coordinates_pwr_long(void);
void power_ok(void);
void power_esc(void);
void coordinates_ok(void);
void save_device_up(void);
void save_device_down(void);
void save_device_ok(void);
void save_device_esc(void);
void delete_device_ok(void);
void set_device_number_up(void);
void set_device_number_down(void);
void set_device_number_ok(void);
void set_device_number_esc(void);
void set_device_id_up(void);
void set_device_id_down(void);
void set_device_id_ok(void);
void set_device_id_esc(void);
void set_freq_channel_up(void);
void set_freq_channel_down(void);
void set_freq_channel_ok(void);
void set_freq_channel_esc(void);
void set_tx_power_up(void);
void set_tx_power_down(void);
void set_tx_power_ok(void);
void set_tx_power_esc(void);
void set_update_interval_up(void);
void set_update_interval_down(void);
void set_update_interval_ok(void);
void set_update_interval_esc(void);
void set_timeout_up(void);
void set_timeout_down(void);
void set_timeout_ok(void);
void set_timeout_esc(void);
void set_fence_up(void);
void set_fence_down(void);
void set_fence_ok(void);
void set_fence_esc(void);
void set_timezone_up(void);
void set_timezone_down(void);
void set_timezone_ok(void);
void set_timezone_esc(void);
void time_zone_inc(void);
void time_zone_dec(void);
void confirm_settings_save_ok(void);
void confirm_settings_save_esc(void);



//ALL MENUS HERE
enum
{
    //menu number starts from 1, because 0 is used as "end marker" in menu structs
    M_MAIN = 1,
    M_NAVIGATION,
	M_COORDINATES,
	M_DEVICES,
	M_POWER,
	M_DEVICE_SUBMENU,
	M_SAVE_DEVICE,
	M_SAVED_POPUP,
	M_DELETE_DEVICE,
	M_DELETED_POPUP,
	M_SETTINGS,
	M_EDIT_DEVICE,
	M_EDIT_RADIO,
	M_EDIT_OTHER,
	M_SET_DEVICE_NUMBER,
    M_SET_DEVICE_ID,
    M_SET_FREQ_CHANNEL,
    M_SET_TX_POWER,
	M_SET_UPDATE_INTERVAL,
	M_SET_TIMEOUT,
	M_SET_FENCE,
	M_SET_TIMEZONE,
	M_CONFIRM_SETTINGS_SAVE
};



//ALL MENU ITEMS HERE (for each menu separately)
//note: for all menus first item always has index of 0
#define M_ALL_I_FIRST (0)



//MAIN
enum
{
    M_MAIN_I_NAVIGATION = 0,
	M_MAIN_I_DEVICES,
	M_MAIN_I_SETTINGS,					//last item
    M_MAIN_I_LAST = M_MAIN_I_SETTINGS	//copy last item here
};



//DEVICES
enum
{
    M_DEVICES_I_DEVICES = 0,
	M_DEVICES_I_POINTS,					//last item
    M_DEVICES_I_LAST = M_DEVICES_I_POINTS	//copy last item here
};



//POWER SUBMENU
enum
{
    M_POWER_I_ALARM = 0,
	M_POWER_I_SOUND,
	M_POWER_I_POWEROFF,						//last item
	M_POWER_I_LAST = M_POWER_I_POWEROFF		//copy last item here
};



//DEVICE SUBMENU
enum
{
	M_DEVICE_SUBMENU_I_SAVE = 0,
	M_DEVICE_SUBMENU_I_DELETE,
	M_DEVICE_SUBMENU_I_LAST = M_DEVICE_SUBMENU_I_DELETE
};



//SETTINGS
enum
{
    M_SETTINGS_I_DEVICE = 0,
	M_SETTINGS_I_RADIO,
	M_SETTINGS_I_OTHER,							//last item
    M_SETTINGS_I_LAST = M_SETTINGS_I_OTHER		//copy last item here
};



//EDIT DEVICE
enum
{
    M_EDIT_DEVICE_I_DEVICE_NUMBER = 0,
	M_EDIT_DEVICE_I_DEVICE_ID,								//last item
	M_EDIT_DEVICE_I_LAST = M_EDIT_DEVICE_I_DEVICE_ID		//copy last item here
};



//EDIT RADIO
enum
{
	M_EDIT_RADIO_I_UPDATE_INTERVAL = 0,
    M_EDIT_RADIO_I_FREQ_CHANNEL,
	M_EDIT_RADIO_I_TX_POWER,								//last item
	M_EDIT_RADIO_I_LAST = M_EDIT_RADIO_I_TX_POWER		//copy last item here
};



//EDIT OTHER
enum
{
	M_EDIT_OTHER_I_TIMEOUT = 0,
	M_EDIT_OTHER_I_FENCE,								//last item
	M_EDIT_OTHER_I_TIMEZONE,
	M_EDIT_OTHER_I_LAST = M_EDIT_OTHER_I_TIMEZONE		//copy last item here
};



//Only exclusive (non default) actions here, for example edit a variable in settings
const struct
{
    uint8_t current_menu;
    uint8_t button_pressed;
    void (*execute_function)(void);
} menu_exclusive_table[] = 
{
//  Current Menu                Button pressed          Action (function name)
	{M_NAVIGATION,				BTN_OK,					navigation_ok},
	{M_NAVIGATION,				BTN_UP,					navigation_and_coordinates_up},
	{M_NAVIGATION,				BTN_DOWN,				navigation_and_coordinates_down},
	{M_NAVIGATION,				BTN_ESC,				navigation_esc},
	{M_COORDINATES,				BTN_OK,					coordinates_ok},
	{M_COORDINATES,				BTN_UP,					navigation_and_coordinates_up},
	{M_COORDINATES,				BTN_DOWN,				navigation_and_coordinates_down},
	{M_COORDINATES,				BTN_ESC,				coordinates_esc},
	{M_MAIN,					BTN_OK,					main_ok},
	{M_MAIN,					BTN_PWR_LONG,			main_navigation_coordinates_pwr_long},
	{M_NAVIGATION,				BTN_PWR_LONG,			main_navigation_coordinates_pwr_long},
	{M_COORDINATES,				BTN_PWR_LONG,			main_navigation_coordinates_pwr_long},
	{M_POWER,					BTN_OK,					power_ok},
	{M_POWER,					BTN_ESC,				power_esc},
	{M_SAVE_DEVICE,				BTN_UP,					save_device_up},
	{M_SAVE_DEVICE,				BTN_DOWN,				save_device_down},
	{M_SAVE_DEVICE,				BTN_OK,					save_device_ok},
	{M_SAVE_DEVICE,				BTN_ESC,				save_device_esc},
	{M_DELETE_DEVICE,			BTN_OK,					delete_device_ok},
	{M_SET_DEVICE_NUMBER,		BTN_UP,                 set_device_number_up},
	{M_SET_DEVICE_NUMBER,		BTN_DOWN,               set_device_number_down},
	{M_SET_DEVICE_NUMBER,		BTN_OK,                 set_device_number_ok},
	{M_SET_DEVICE_NUMBER,		BTN_ESC,            	set_device_number_esc},
	{M_SET_DEVICE_ID,			BTN_UP,                 set_device_id_up},
	{M_SET_DEVICE_ID,			BTN_DOWN,               set_device_id_down},
	{M_SET_DEVICE_ID,			BTN_OK,                 set_device_id_ok},
	{M_SET_DEVICE_ID,			BTN_ESC,            	set_device_id_esc},
	{M_SET_UPDATE_INTERVAL,		BTN_UP,                 set_update_interval_up},
	{M_SET_UPDATE_INTERVAL,		BTN_DOWN,               set_update_interval_down},
	{M_SET_UPDATE_INTERVAL,		BTN_OK,                 set_update_interval_ok},
	{M_SET_UPDATE_INTERVAL,		BTN_ESC,            	set_update_interval_esc},
	{M_SET_FREQ_CHANNEL,		BTN_UP,                 set_freq_channel_up},
	{M_SET_FREQ_CHANNEL,		BTN_DOWN,               set_freq_channel_down},
	{M_SET_FREQ_CHANNEL,		BTN_OK,                 set_freq_channel_ok},
	{M_SET_FREQ_CHANNEL,		BTN_ESC,            	set_freq_channel_esc},
	{M_SET_TX_POWER,			BTN_UP,                 set_tx_power_up},
	{M_SET_TX_POWER,			BTN_DOWN,               set_tx_power_down},
	{M_SET_TX_POWER,			BTN_OK,                 set_tx_power_ok},
	{M_SET_TX_POWER,			BTN_ESC,            	set_tx_power_esc},
	{M_SET_TIMEOUT,				BTN_UP,                 set_timeout_up},
	{M_SET_TIMEOUT,				BTN_DOWN,               set_timeout_down},
	{M_SET_TIMEOUT,				BTN_OK,                 set_timeout_ok},
	{M_SET_TIMEOUT,				BTN_ESC,            	set_timeout_esc},
	{M_SET_FENCE,				BTN_UP,                 set_fence_up},
	{M_SET_FENCE,				BTN_DOWN,               set_fence_down},
	{M_SET_FENCE,				BTN_OK,                 set_fence_ok},
	{M_SET_FENCE,				BTN_ESC,            	set_fence_esc},
	{M_SET_TIMEZONE,			BTN_UP,					set_timezone_up},
	{M_SET_TIMEZONE,			BTN_DOWN,				set_timezone_down},
	{M_SET_TIMEZONE,			BTN_OK,					set_timezone_ok},
	{M_SET_TIMEZONE,			BTN_ESC,				set_timezone_esc},
	{M_CONFIRM_SETTINGS_SAVE,	BTN_OK,					confirm_settings_save_ok},
	{M_CONFIRM_SETTINGS_SAVE,	BTN_ESC,				confirm_settings_save_esc},
    {0, 0, 0}   //end marker
};



//Defaul behaviour (non exclusive) when OK button has been pressed (move forward)
const struct
{
    uint8_t current_menu;
    uint8_t current_item;
    uint8_t next_menu;
} menu_forward_table[] = 
{
//  Current Menu                Current Item                		Next Menu
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_SAVE,			M_SAVE_DEVICE},
	{M_DEVICE_SUBMENU,			M_DEVICE_SUBMENU_I_DELETE,			M_DELETE_DEVICE},
	{M_SETTINGS,				M_SETTINGS_I_DEVICE,				M_EDIT_DEVICE},
	{M_SETTINGS,				M_SETTINGS_I_RADIO,					M_EDIT_RADIO},
	{M_SETTINGS,				M_SETTINGS_I_OTHER,					M_EDIT_OTHER},
	{M_EDIT_DEVICE,				M_EDIT_DEVICE_I_DEVICE_NUMBER,		M_SET_DEVICE_NUMBER},
	{M_EDIT_DEVICE,				M_EDIT_DEVICE_I_DEVICE_ID,			M_SET_DEVICE_ID},
	{M_EDIT_RADIO,				M_EDIT_RADIO_I_UPDATE_INTERVAL,		M_SET_UPDATE_INTERVAL},
	{M_EDIT_RADIO,				M_EDIT_RADIO_I_FREQ_CHANNEL,		M_SET_FREQ_CHANNEL},
	{M_EDIT_RADIO,				M_EDIT_RADIO_I_TX_POWER,			M_SET_TX_POWER},
	{M_EDIT_OTHER,				M_EDIT_OTHER_I_TIMEOUT,				M_SET_TIMEOUT},
	{M_EDIT_OTHER,				M_EDIT_OTHER_I_FENCE,				M_SET_FENCE},
	{M_EDIT_OTHER,				M_EDIT_OTHER_I_TIMEZONE,			M_SET_TIMEZONE},
    {0, 0, 0}   //end marker
};



//Defaul behaviour (non exclusive) when ESC button has been pressed (move backward)
const struct
{
    uint8_t current_menu;
    uint8_t next_menu;
} menu_backward_table[] = 
{
//  Current Menu                Next Menu
    {M_NAVIGATION,              M_MAIN},
	{M_COORDINATES,				M_NAVIGATION},
	{M_DEVICES,                 M_MAIN},
	{M_DEVICE_SUBMENU,     		M_COORDINATES},
	{M_DELETE_DEVICE,			M_DEVICE_SUBMENU},
	{M_SAVED_POPUP,				M_COORDINATES},
	{M_DELETED_POPUP,			M_COORDINATES},
    {M_SETTINGS,                M_CONFIRM_SETTINGS_SAVE},
    {M_EDIT_DEVICE,             M_SETTINGS},
	{M_EDIT_RADIO,	            M_SETTINGS},
	{M_EDIT_OTHER,              M_SETTINGS},
    {0, 0}      //end marker
};



//Struct with list of menus and real-time values of current item in current menu. Last Item is needed for scroll function
//note: if current menu has no items (like INFO menu) no need to put it in structure below, because item functions (get, get last, set) automatically return 0 (which is zero item)
struct
{
    const uint8_t curent_menu;
    uint8_t cur_item;
    const uint8_t last_item;
} item_table[] = 
{
//  Current Menu                Current Item                Last Item in Current Menu
    {M_MAIN,                    M_ALL_I_FIRST,              M_MAIN_I_LAST},
	{M_DEVICES,					M_ALL_I_FIRST,              M_DEVICES_I_LAST},
	{M_POWER,					M_ALL_I_FIRST,				M_POWER_I_LAST},
	{M_DEVICE_SUBMENU,			M_ALL_I_FIRST,				M_DEVICE_SUBMENU_I_LAST},
    {M_SETTINGS,                M_ALL_I_FIRST,              M_SETTINGS_I_LAST},
	{M_EDIT_DEVICE,				M_ALL_I_FIRST,				M_EDIT_DEVICE_I_LAST},
	{M_EDIT_RADIO,				M_ALL_I_FIRST,				M_EDIT_RADIO_I_LAST},
	{M_EDIT_OTHER,				M_ALL_I_FIRST,				M_EDIT_OTHER_I_LAST},
    {0, 0, 0}   //end marker
};



//List of menus with appropriate functions to draw it (show on screen)
const struct
{
    uint8_t current;
    void (*action)(void);
} menu_draw_table[] = 
{
//  Current Menu                Draw Function
	{M_NAVIGATION,				draw_navigation},
	{M_COORDINATES,				draw_coordinates},
	{M_MAIN,                    draw_main},
	{M_DEVICES,                 draw_devices},
	{M_POWER,					draw_power},
	{M_DEVICE_SUBMENU,     		draw_device_submenu},
	{M_SAVE_DEVICE,				draw_save_device},
	{M_DELETE_DEVICE,			draw_delete_device},
	{M_SAVED_POPUP,				draw_saved_popup},
	{M_DELETED_POPUP,			draw_deleted_popup},
    {M_SETTINGS,                draw_settings},
	{M_EDIT_DEVICE,				draw_edit_device},
	{M_EDIT_RADIO,				draw_edit_radio},
	{M_EDIT_OTHER,				draw_edit_other},
    {M_SET_DEVICE_NUMBER,		draw_set_device_number},
    {M_SET_DEVICE_ID,			draw_set_device_id},
	{M_SET_UPDATE_INTERVAL,		draw_set_update_interval},
    {M_SET_FREQ_CHANNEL,		draw_set_freq_channel},
    {M_SET_TX_POWER,			draw_set_tx_power},
	{M_SET_TIMEOUT,				draw_set_timeout},
	{M_SET_FENCE,				draw_set_fence},
	{M_SET_TIMEZONE,			draw_set_timezone},
	{M_CONFIRM_SETTINGS_SAVE,	draw_confirm_settings_save},
    {0, 0}      //end marker
};



uint8_t current_menu;                               		//Actually Current Menu value (real-time)
uint8_t return_from_power_menu; 							//Menu to return to after exit from power menu. Power menu can be accessed from different menus, therefore we have to store menu to return.
char tmp_buf[16];                                   		//temporary char buffer for screen text fragments
uint8_t flag_settings_changed = 0;							//Have settings been changed?
uint8_t current_mem_point_to_save = MEMORY_POINT_FIRST;		//Current number of memory point to save

struct settings_struct *p_settings;
struct settings_struct settings_copy;

uint8_t *p_update_interval_values;
uint8_t *p_tx_power_values;
struct gps_raw_struct *p_gps_raw;
struct gps_num_struct *p_gps_num;

uint8_t this_device;								//device number of this device, see init_menu()
uint8_t navigate_to_device;							//a device number that we are navigating to right now


struct devices_struct **pp_devices;



//Init and show MAIN menu
void init_menu(void)
{
	//Load settings and create a local copy
	p_settings = get_settings();
	settings_copy = *p_settings;
	this_device = p_settings->device_number;
	navigate_to_device = this_device;

	//Load all devices
	pp_devices = get_devices();

	//Load other
	p_tx_power_values = get_tx_power_values();
	p_update_interval_values = get_update_interval_values();
	p_gps_raw = get_gps_raw();
	p_gps_num = get_gps_num();

	//Init start menu
    current_menu = M_MAIN;
    return_from_power_menu = M_MAIN;
    set_current_item(M_MAIN_I_NAVIGATION);
    draw_current_menu();

    //Create fake devices
//    pp_devices[this_device]->exist_flag = 1;
//    pp_devices[this_device]->memory_point_flag = 0;
//    pp_devices[this_device]->latitude.as_float = 59.907968;	//0
//    pp_devices[this_device]->longitude.as_float = 30.455612;
//
//
//    pp_devices[p_settings->devices_on_air]->exist_flag = 1;
//    pp_devices[p_settings->devices_on_air]->device_id = 'F';
//    pp_devices[p_settings->devices_on_air]->memory_point_flag = 0;
//    pp_devices[p_settings->devices_on_air]->latitude.as_float = 59.906175;
//    pp_devices[p_settings->devices_on_air]->longitude.as_float = 30.457254;
//
//
//    pp_devices[p_settings->devices_on_air - 1]->exist_flag = 1;
//    pp_devices[p_settings->devices_on_air - 1]->device_id = 'Z';
//    pp_devices[p_settings->devices_on_air - 1]->memory_point_flag = 0;
//    pp_devices[p_settings->devices_on_air - 1]->latitude.as_float = 59.909516;
//    pp_devices[p_settings->devices_on_air - 1]->longitude.as_float = 30.462170;
}



//Check for buttons and change menu if needed
void change_menu(uint8_t button_code)
{
	if (lcd_get_display_status() == LCD_DISPLAY_ON) //if lcd is on
	{
		//search for exclusive operation for this case
		for (uint8_t i = 0; menu_exclusive_table[i].current_menu; i++)     //until end marker
		{
			if (current_menu == menu_exclusive_table[i].current_menu &&
				button_code == menu_exclusive_table[i].button_pressed)
			{
				menu_exclusive_table[i].execute_function();
				return;         //exit
			}
		}

		//well, there is no exclusive operations for that case, perform default action
		switch (button_code)
		{
			case BTN_UP:
				scroll_up();
				break;

			case BTN_DOWN:
				scroll_down();
				break;

			case BTN_OK:
				switch_forward();
				break;

			case BTN_ESC:
				switch_backward();
				break;

			case BTN_PWR:
				lcd_display_off_request();
				if (current_menu == M_NAVIGATION)
				{
					timer4_stop(); //stop compass
				}
				break;

			default:
				break;
		}
	}
	else if (button_code == BTN_PWR_LONG)	//if lcd is off then check for PRW button was pressed. If so - toggle the lcd
	{
		if (current_menu == M_NAVIGATION)
		{
			timer4_start(); //start compass
		}
		lcd_display_on();
	}
}



//Scroll current menu Up
void scroll_up(void)
{
    uint8_t current = get_current_item();
    uint8_t last = get_last_item();
    
    if (current == M_ALL_I_FIRST)
    {
        set_current_item(last);
    }
    else
    {
        set_current_item(current - 1);
    }
    
    //draw_current_menu();
}



//Scroll current menu Down
void scroll_down(void)
{
    uint8_t current = get_current_item();
    uint8_t last = get_last_item();
    
    if (current == last)
    {
        set_current_item(M_ALL_I_FIRST);
    }
    else
    {
        set_current_item(current + 1);
    }
    
    //draw_current_menu();
}



//Switch menu forward by default
void switch_forward(void)
{
    for (uint8_t i = 0; menu_forward_table[i].current_menu; i++)
    {
        if (current_menu == menu_forward_table[i].current_menu &&
            get_current_item() == menu_forward_table[i].current_item)
        {
            current_menu = menu_forward_table[i].next_menu;
            break;
        }
    }
    //draw_current_menu();
}



//Switch menu backward by default
void switch_backward(void)
{
    for (uint8_t i = 0; menu_backward_table[i].current_menu; i++)
    {
        if (current_menu == menu_backward_table[i].current_menu)
        {
            set_current_item(M_ALL_I_FIRST);        //reset current item before exit
            current_menu = menu_backward_table[i].next_menu;
            break;
        }
    }
    //draw_current_menu();
}



//Get currently selected item in current menu
uint8_t get_current_item(void)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            return item_table[i].cur_item;
        }
    }
    return 0;       //automatically return 0 if item not found in item_table[]
}



//Get last item in current menu
uint8_t get_last_item(void)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            return item_table[i].last_item;
        }
    }
    return 0;       //automatically return 0 if item not found in item_table[]
}



//Set item to be current in current menu
void set_current_item(uint8_t new_value)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (current_menu == item_table[i].curent_menu)
        {
            item_table[i].cur_item = new_value;
            break;
        }
    }
}



//Reset item in any menu
void reset_current_item_in_menu(uint8_t menu)
{
    for (uint8_t i = 0; item_table[i].curent_menu; i++)
    {
        if (menu == item_table[i].curent_menu)
        {
            item_table[i].cur_item = M_ALL_I_FIRST;
            break;
        }
    }
}



//--------------------------------------------------------------
//--------------------------- DRAW -----------------------------
//--------------------------------------------------------------



//Draw current menu
void draw_current_menu(void)
{
    for (uint8_t i = 0; menu_draw_table[i].current; i++)
    {
        if (current_menu == menu_draw_table[i].current)
        {
            menu_draw_table[i].action();
            break;
        }
    }
}



//MAIN
void draw_main(void)
{
    #define MAIN_ROW               (1)
    #define MAIN_COL               (1)
    
    lcd_clear();
    lcd_print(0, MAIN_COL, "MENU", 0);
    lcd_print(MAIN_ROW, MAIN_COL, "Navigation", 0);
    lcd_print(MAIN_ROW + 1, MAIN_COL, "Devices", 0);
    lcd_print(MAIN_ROW + 2, MAIN_COL, "Settings", 0);
    lcd_print(MAIN_ROW + get_current_item(), MAIN_COL - 1, ">", 0);
    draw_bat_level();

    lcd_update();
}



//Draw an icon in top right corner
void draw_bat_level(void)
{
	lcd_char_pos(0, 13, SYMB8_BAT_TAIL, 0);
	lcd_char_pos(0, 14, SYMB8_BAT_MID, 0);
	lcd_char_pos(0, 15, SYMB8_BAT_HEAD, 0);

	for (uint8_t px = 0; px < (get_battery_level() + 1); px++)
	{
		lcd_byte2buf(108 + px, 0xF8);	//1st line, 108 is a start pixel position
		lcd_byte2buf((128 + 108) + px, 0x0F);	//2nd line
	}
}



void draw_devices(void)
{
	lcd_clear();

	if (get_current_item() == M_DEVICES_I_DEVICES)	//draw all online devices
	{
		uint8_t active_devs = 0;
		uint8_t active_row = 0;

		for (uint8_t dev = DEVICE_NUMBER_FIRST; dev < DEVICE_NUMBER_LAST + 1; dev++)
		{
			if (pp_devices[dev]->exist_flag == 1)
			{
				if (dev != this_device)
				{
					active_devs++;
					itoa32(dev, &tmp_buf[0]);
					lcd_print(active_row, 0, tmp_buf, 0);
					lcd_char_pos(active_row, 1, pp_devices[dev]->device_id, 0);
					active_row++;
				}
			}
		}

		if (active_devs == 0)
		{
			lcd_print(1, 3, "No active", 0);
			lcd_print(2, 4, "devices", 0);
		}
	}
	else if (get_current_item() == M_DEVICES_I_POINTS)	//draw all saved points
	{
		uint8_t active_pts = 0;
		uint8_t active_row = 0;

		for (uint8_t pt = MEMORY_POINT_FIRST; pt < MEMORY_POINT_LAST + 1; pt++)
		{
			if (pp_devices[pt]->exist_flag == 1)
			{
				active_pts++;
				lcd_print(active_row, 0, get_memory_point_name(pt), 0);
				active_row++;
			}
		}

		if (active_pts == 0)
		{
			lcd_print(1, 3, "No active", 0);
			lcd_print(2, 4, "points", 0);
		}
	}

//	lcd_print(0, 0, "1A  123 NWW   AF", 0);
//	lcd_print(1, 0, "2B  1.3 N    T  ", 0);
//	lcd_print(2, 0, "3C   27 SE    A ", 0);
//	lcd_print(3, 0, "5E 10.1 W      F", 0);

	lcd_update();
}



void draw_power(void)
{

	#define EDIT_POWER_ROW               (1)
	#define EDIT_POWER_COL               (1)

	lcd_clear();
	lcd_print(0, EDIT_POWER_COL + 4, "POWER", 0);

    lcd_print(EDIT_POWER_ROW, EDIT_POWER_COL, "Alarm is ", 0);
    if (get_my_alarm_status())
    {
    	lcd_print_next("On", 0);
    }
    else
    {
    	lcd_print_next("Off", 0);
    }

    lcd_print(EDIT_POWER_ROW + 1, EDIT_POWER_COL, "Sound is ", 0);
    if (get_sound_status())
    {
    	lcd_print_next("On", 0);
    }
    else
    {
    	lcd_print_next("Off", 0);
    }

    lcd_print(EDIT_POWER_ROW + 2, EDIT_POWER_COL, "Power Off", 0);
    lcd_print(EDIT_POWER_ROW + get_current_item(), EDIT_POWER_COL - 1, ">", 0);

    draw_bat_level();

    lcd_update();
}



//NAVIGATION
void draw_navigation(void)
{
    if (pp_devices[navigate_to_device]->exist_flag == 0)
    {
    	navigate_to_device = this_device;		//reset navigate to device
    }

	//Spoke circle
	lcd_clear();
	lcd_bitmap(&spoke_blank[0]);

	//Draw arrow (my course)
	#define SCREEN_CENTER_X	(31)
	#define SCREEN_CENTER_Y	(31)
	#define ARROW_LEN		(15)

	if (is_north_ready())	//only when north value is available
	{
		float x1, y1;
		float arrow_ang_rad;

		arrow_ang_rad = get_north();

		x1 = SCREEN_CENTER_X + ARROW_LEN * sin(arrow_ang_rad);
		y1 = SCREEN_CENTER_Y - ARROW_LEN * cos(arrow_ang_rad);

		lcd_draw_line(SCREEN_CENTER_X, SCREEN_CENTER_Y, x1, y1);
	}
	else	//print a center dot
	{
		lcd_draw_dot(SCREEN_CENTER_X, SCREEN_CENTER_Y);
	}


	//GPS fix status
    if (p_gps_num->status == GPS_DATA_VALID)
    {
        if (p_gps_num->mode == GPS_POSITION_2DFIX)
        {
            lcd_print(0, 11, "2D", 0);
        }
        else if (p_gps_num->mode == GPS_POSITION_3DFIX)
        {
            lcd_print(0, 11, "3D", 0);
        }
        else
        {
            lcd_print(0, 11, "NO", 0);
        }
    }
    else
    {
        lcd_print(0, 11, "NA", 0);
    }


	//print ID
	lcd_char16_pos(3, 8, pp_devices[navigate_to_device]->device_id, 0);


    if (navigate_to_device == this_device)		//if navigate to this device
    {
    	if (pp_devices[navigate_to_device]->timeout_flag)
		{
			lcd_char_pos(0, 8, SYMB8_TIMEOUT, 0);
		}

    	if (pp_devices[navigate_to_device]->alarm_flag)
    	{
    		lcd_char_pos(0, 10, SYMB8_ALARM, 0);
    	}

    	itoa32(p_gps_num->hour_tz, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
        lcd_print(1, 8, &tmp_buf[0], 0);
        lcd_print_next(":", 0);

    	itoa32(p_gps_num->minute_tz, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
        lcd_print_next(&tmp_buf[0], 0);
        lcd_print_next(":", 0);

    	itoa32(p_gps_num->second, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
        lcd_print_next(&tmp_buf[0], 0);


    	itoa32(p_gps_num->day_tz, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
        lcd_print(2, 8, &tmp_buf[0], 0);
        lcd_print_next(".", 0);

    	itoa32(p_gps_num->month_tz, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
    	lcd_print_next(&tmp_buf[0], 0);
        lcd_print_next(".", 0);

    	itoa32(p_gps_num->year_tz, &tmp_buf[0]);
    	add_leading_zero(&tmp_buf[0]);
        lcd_print_next(&tmp_buf[0], 0);

//    	ssd1306_char16_pos(3, 14, SYMB16_TIMEOUT, 0);	//todo: those icons are not needed at all anymore
//    	ssd1306_char16_pos(3, 12, SYMB16_ALARM, 0);
//    	ssd1306_char16_pos(3, 10, SYMB16_FENCE, 0);

        itoa32(p_gps_num->sat_view, &tmp_buf[0]);	//satellites in use|view
		lcd_print_viceversa(3, 15, &tmp_buf[0], 0);
		lcd_print_next_viceversa("/", 0);
		itoa32(p_gps_num->sat_used, &tmp_buf[0]);
		lcd_print_next_viceversa(&tmp_buf[0], 0);

//		lcd_char_pos(3, 15, 'm', 0); 				//meter char
//		itoa32((int32_t)(p_gps_num->altitude), &tmp_buf[0]);
//		lcd_print_viceversa(3, 14, &tmp_buf[0], 0);

    }
    else										//if navigate to another device
    {
    	if (pp_devices[navigate_to_device]->memory_point_flag == 0)
    	{
    		if (pp_devices[navigate_to_device]->timeout_flag)
    		{
    			lcd_char_pos(0, 8, SYMB8_TIMEOUT, 0);
    		}
    		else
    		{
    			lcd_char_pos(0, 8, pp_devices[navigate_to_device]->rx_icon, 0); //Print RX icon only for real devices todo: delete
    		}
    	}

    	if (pp_devices[navigate_to_device]->fence_flag)
    	{
    		lcd_char_pos(0, 9, SYMB8_FENCE, 0);
    	}

    	if (pp_devices[navigate_to_device]->alarm_flag)
    	{
    		lcd_char_pos(0, 10, SYMB8_ALARM, 0);
    	}

		//Draw notch
		#define NOTCH_START		(21)
		#define NOTCH_LEN		(8)

    	if (pp_devices[navigate_to_device]->distance != 0)
		{
			float sx, sy, fx, fy;
			float notch_ang_rad = pp_devices[navigate_to_device]->heading_rad;

			sx = SCREEN_CENTER_X + NOTCH_START * sin(notch_ang_rad);
			sy = SCREEN_CENTER_Y - NOTCH_START * cos(notch_ang_rad);

			fx = SCREEN_CENTER_X + (NOTCH_START + NOTCH_LEN) * sin(notch_ang_rad);
			fy = SCREEN_CENTER_Y - (NOTCH_START + NOTCH_LEN) * cos(notch_ang_rad);

			lcd_draw_line(sx, sy, fx, fy);
		}

		//print distance
		convert_main_distance(pp_devices[navigate_to_device]->distance, &tmp_buf[0]);
		lcd_print16_viceversa(1, 14, &tmp_buf[0], 0);

		//print heading
		lcd_char_pos(2, 15, SYMB8_DEGREE, 0); //degree char
		itoa32(pp_devices[navigate_to_device]->heading_deg, &tmp_buf[0]);
		lcd_print_viceversa(2, 14, &tmp_buf[0], 0);

		//print altitude diff
		lcd_char_pos(3, 15, 'm', 0); //meter char
		convert_main_alt_difference(pp_devices[navigate_to_device]->delta_altitude, &tmp_buf[0]);
		lcd_print_viceversa(3, 14, &tmp_buf[0], 0);
		if (pp_devices[navigate_to_device]->delta_altitude > 0)
		{
			lcd_char('+', 0);
		}
    }

    draw_bat_level();

	lcd_update();

}



//COORDINATES
void draw_coordinates(void)
{
    lcd_clear();

    lcd_print(0, 0, "#", 0);
    itoa32(navigate_to_device, &tmp_buf[0]);
    lcd_print_next(&tmp_buf[0], 0);

    if (pp_devices[navigate_to_device]->memory_point_flag == 1)
    {
		lcd_print_next(" POINT ", 0);
		lcd_print_next(get_memory_point_name(navigate_to_device), 0);

		//save date
		itoa32(pp_devices[navigate_to_device]->save_day, &tmp_buf[0]);
		add_leading_zero(&tmp_buf[0]);
		lcd_print(3, 11, &tmp_buf[0], 0);
		lcd_print_next(".", 0);

		itoa32(pp_devices[navigate_to_device]->save_month, &tmp_buf[0]);
		add_leading_zero(&tmp_buf[0]);
		lcd_print_next(&tmp_buf[0], 0);
    }
    else
    {
		lcd_print(0, 3, "ID", 0);
		lcd_char_pos(0, 6, pp_devices[navigate_to_device]->device_id, 0);

		if (navigate_to_device == this_device)
		{
			lcd_print(0, 8, "(YOU)", 0);
		}

		//print timeout here for all
		convert_timeout(pp_devices[navigate_to_device]->timeout, &tmp_buf[0]);
		lcd_print(3, 10, &tmp_buf[0], 0);
    }

    lcd_print(1, 0, "LAT", 0);
    ftoa32(pp_devices[navigate_to_device]->latitude.as_float, 6, &tmp_buf[0]);
    lcd_print_viceversa(1, 14, &tmp_buf[0], 0);
    if (pp_devices[navigate_to_device]->latitude.as_float >= 0)
    {
        lcd_char('+', 0);
    }
    lcd_char_pos(1, 15, SYMB8_DEGREE, 0); //degree char


    lcd_print(2, 0, "LON", 0);
    ftoa32(pp_devices[navigate_to_device]->longitude.as_float, 6, &tmp_buf[0]);
    lcd_print_viceversa(2, 14, &tmp_buf[0], 0);
    if (pp_devices[navigate_to_device]->longitude.as_float >= 0)
    {
        lcd_char('+', 0);
    }
    lcd_char_pos(2, 15, SYMB8_DEGREE, 0); //degree char


    //print ALT here for all
    lcd_print(3, 0, "ALT", 0);
    itoa32(pp_devices[navigate_to_device]->altitude.as_integer, &tmp_buf[0]);
    lcd_print(3, 4, &tmp_buf[0], 0);
    lcd_char('m', 0); //meter char

    draw_bat_level();

    lcd_update();
}



//DEVICE SUBMENU
void draw_device_submenu(void)
{
	#define DEVICE_SUBMENU_ROW               (1)
	#define DEVICE_SUBMENU_COL               (1)

	lcd_clear();

	lcd_print(0, DEVICE_SUBMENU_COL, "DEVICE #", 0);
    itoa32(navigate_to_device, &tmp_buf[0]);
    lcd_print_next(&tmp_buf[0], 0);

	lcd_print(DEVICE_SUBMENU_ROW, DEVICE_SUBMENU_COL, "Save", 0);
	lcd_print(DEVICE_SUBMENU_ROW + 1, DEVICE_SUBMENU_COL, "Delete", 0);
	lcd_print(DEVICE_SUBMENU_ROW + get_current_item(), DEVICE_SUBMENU_COL - 1, ">", 0);

	lcd_update();
}



//SAVE DEVICE
void draw_save_device(void)
{
	lcd_clear();

	lcd_print(0, 2, "Save #", 0);

    itoa32(navigate_to_device, &tmp_buf[0]);
    lcd_print_next(&tmp_buf[0], 0);
    lcd_print_next(" as:", 0);

    lcd_print(1, 5, get_memory_point_name(current_mem_point_to_save), 0);

    lcd_print(2, 3, "OK save", 0);
    lcd_print(3, 3, "ESC cancel", 0);

	lcd_update();
}



//DELETE DEVICE
void draw_delete_device(void)
{
	lcd_clear();

	lcd_print(0, 3, "Delete #", 0);

    itoa32(navigate_to_device, &tmp_buf[0]);
    lcd_print_next(&tmp_buf[0], 0);
    lcd_print_next("?", 0);

    lcd_print(2, 3, "OK delete", 0);
    lcd_print(3, 3, "ESC cancel", 0);

	lcd_update();
}



//SAVED POPUP
void draw_saved_popup(void)
{
	lcd_clear();
	lcd_print(0, 4, "Saved!", 0);
    lcd_print(2, 3, "ESC close", 0);
	lcd_update();
}



//DELETED POPUP
void draw_deleted_popup(void)
{
	lcd_clear();
	lcd_print(0, 3, "Deleted!", 0);
    lcd_print(2, 3, "ESC close", 0);
	lcd_update();
}



//SETTINGS
void draw_settings(void)
{
    #define SETTINGS_ROW               (1)
    #define SETTINGS_COL               (1)
    
    lcd_clear();
    lcd_print(0, SETTINGS_COL, "SETTINGS", 0);
    lcd_print(SETTINGS_ROW, SETTINGS_COL, "Device", 0);
    lcd_print(SETTINGS_ROW + 1, SETTINGS_COL, "Radio", 0);
    lcd_print(SETTINGS_ROW + 2, SETTINGS_COL, "Other", 0);
    lcd_print(SETTINGS_ROW + get_current_item(), SETTINGS_COL - 1, ">", 0);
    lcd_update();
}



//EDIT DEVICE SETTINGS
void draw_edit_device(void)
{
	#define EDIT_DEVICE_ROW               (1)
	#define EDIT_DEVICE_COL               (1)

    lcd_clear();
    lcd_print(0, EDIT_DEVICE_COL, "EDIT DEVICE", 0);

    lcd_print(EDIT_DEVICE_ROW, EDIT_DEVICE_COL, "Number", 0);
    itoa32(settings_copy.device_number, &tmp_buf[0]);
    lcd_print(EDIT_DEVICE_ROW, EDIT_DEVICE_COL + 10, &tmp_buf[0], 0);
    lcd_print(EDIT_DEVICE_ROW, EDIT_DEVICE_COL + 11, "/", 0);
    itoa32(settings_copy.devices_on_air, &tmp_buf[0]);
    lcd_print(EDIT_DEVICE_ROW, EDIT_DEVICE_COL + 12, &tmp_buf[0], 0);

    lcd_print(EDIT_DEVICE_ROW + 1, EDIT_DEVICE_COL, "ID", 0);
    tmp_buf[0] = settings_copy.device_id;
    tmp_buf[1] = 0;
	lcd_print(EDIT_DEVICE_ROW + 1, EDIT_DEVICE_COL + 10, &tmp_buf[0], 0);

    lcd_print(EDIT_DEVICE_ROW + get_current_item(), EDIT_DEVICE_COL - 1, ">", 0);
    lcd_update();
}



//EDIT RADIO SETTINGS
void draw_edit_radio(void)
{
	#define EDIT_RADIO_ROW               (1)
	#define EDIT_RADIO_COL               (1)

    lcd_clear();
    lcd_print(0, EDIT_RADIO_COL, "EDIT RADIO", 0);

    lcd_print(EDIT_RADIO_ROW, EDIT_RADIO_COL, "Update", 0);
    itoa32(p_update_interval_values[settings_copy.update_interval_opt], &tmp_buf[0]);
    lcd_print(EDIT_RADIO_ROW, EDIT_RADIO_COL + 10, &tmp_buf[0], 0);
    lcd_print_next("s", 0);

    lcd_print(EDIT_RADIO_ROW + 1, EDIT_RADIO_COL, "Channel", 0);
    itoa32(settings_copy.freq_channel, &tmp_buf[0]);
    lcd_print(EDIT_RADIO_ROW + 1, EDIT_RADIO_COL + 10, &tmp_buf[0], 0);

    lcd_print(EDIT_RADIO_ROW + 2, EDIT_RADIO_COL, "TX Power", 0);
    itoa32(p_tx_power_values[settings_copy.tx_power_opt], &tmp_buf[0]);
    lcd_print(EDIT_RADIO_ROW + 2, EDIT_RADIO_COL + 10, &tmp_buf[0], 0);
    lcd_print_next("mW", 0);

    lcd_print(EDIT_RADIO_ROW + get_current_item(), EDIT_RADIO_COL - 1, ">", 0);
    lcd_update();
}



//EDIT OTHER SETTINGS
void draw_edit_other(void)
{
	#define EDIT_OTHER_ROW               (1)
	#define EDIT_OTHER_COL               (1)

    lcd_clear();
    lcd_print(0, EDIT_OTHER_COL, "EDIT OTHER", 0);

    lcd_print(EDIT_OTHER_ROW, EDIT_OTHER_COL, "Timeout", 0);
    itoa32(settings_copy.timeout_threshold, &tmp_buf[0]);
    lcd_print(EDIT_OTHER_ROW, EDIT_OTHER_COL + 10, &tmp_buf[0], 0);
    lcd_print_next("s", 0);

    lcd_print(EDIT_OTHER_ROW + 1, EDIT_OTHER_COL, "Fence", 0);
    itoa32(settings_copy.fence_threshold, &tmp_buf[0]);
    lcd_print(EDIT_OTHER_ROW + 1, EDIT_OTHER_COL + 10, &tmp_buf[0], 0);
    lcd_print_next("m", 0);

    lcd_print(EDIT_OTHER_ROW + 2, EDIT_OTHER_COL, "Timezone", 0);
	if (settings_copy.time_zone_dir > 0)
	{
		lcd_print(EDIT_OTHER_ROW + 2, EDIT_OTHER_COL + 9, "+", 0);
	}
	else
	{
		lcd_print(EDIT_OTHER_ROW + 2, EDIT_OTHER_COL + 9, "-", 0);
	}

	itoa32(settings_copy.time_zone_hour, &tmp_buf[0]);
	add_leading_zero(&tmp_buf[0]);
	lcd_print_next(&tmp_buf[0], 0);
	lcd_print_next(":", 0);

	itoa32(settings_copy.time_zone_minute, &tmp_buf[0]);
	add_leading_zero(&tmp_buf[0]);
	lcd_print_next(&tmp_buf[0], 0);

    lcd_print(EDIT_OTHER_ROW + get_current_item(), EDIT_OTHER_COL - 1, ">", 0);
    lcd_update();
}



void draw_set_device_number(void)
{
	#define DEVICE_NUMBER_ROW               (2)
	#define DEVICE_NUMBER_COL               (5)

	lcd_clear();
	lcd_print(0, 1, "DEVICE NUMBER", 0);

    itoa32(settings_copy.device_number, &tmp_buf[0]);
    lcd_print(DEVICE_NUMBER_ROW, DEVICE_NUMBER_COL, &tmp_buf[0], 0);

    lcd_print_next(" of ", 0);

    itoa32(settings_copy.devices_on_air, &tmp_buf[0]);
    lcd_print_next(&tmp_buf[0], 0);

	lcd_update();
}



void draw_set_device_id(void)
{
	#define DEVICE_ID_ROW               (2)
	#define DEVICE_ID_COL               (7)

	lcd_clear();
	lcd_print(0, 3, "DEVICE ID", 0);

	tmp_buf[0] = settings_copy.device_id;
	tmp_buf[1] = 0;
	lcd_print(DEVICE_ID_ROW, DEVICE_ID_COL, &tmp_buf[0], 0);

	lcd_update();
}



void draw_set_freq_channel(void)
{
	#define FREQ_CHANNEL_ROW               (2)
	#define FREQ_CHANNEL_COL               (7)

	lcd_clear();
	lcd_print(0, 2, "FREQ CHANNEL", 0);

    itoa32(settings_copy.freq_channel, &tmp_buf[0]);
	lcd_print(FREQ_CHANNEL_ROW, FREQ_CHANNEL_COL, &tmp_buf[0], 0);

	lcd_update();
}



void draw_set_tx_power(void)
{
	#define TX_POWER_ROW               (2)
	#define TX_POWER_COL               (6)

	lcd_clear();
	lcd_print(0, 4, "TX POWER", 0);

    itoa32(p_tx_power_values[settings_copy.tx_power_opt], &tmp_buf[0]);
    lcd_print(TX_POWER_ROW, TX_POWER_COL, &tmp_buf[0], 0);
    lcd_print_next("mW", 0);

	lcd_update();
}



void draw_set_update_interval(void)
{
	#define UPDATE_INTERVAL_ROW               (2)
	#define UPDATE_INTERVAL_COL               (6)

	lcd_clear();
	lcd_print(0, 0, "UPDATE INTERVAL", 0);

    itoa32(p_update_interval_values[settings_copy.update_interval_opt], &tmp_buf[0]);
    lcd_print(UPDATE_INTERVAL_ROW, UPDATE_INTERVAL_COL, &tmp_buf[0], 0);
    lcd_print_next("s", 0);

	lcd_update();
}



void draw_set_timeout(void)
{
	#define TIMEOUT_ROW               (2)
	#define TIMEOUT_COL               (6)

	lcd_clear();
	lcd_print(0, 4, "TIMEOUT", 0);

	itoa32(settings_copy.timeout_threshold, &tmp_buf[0]);
	lcd_print(TIMEOUT_ROW, TIMEOUT_COL, &tmp_buf[0], 0);
	lcd_print_next("s", 0);

	lcd_update();
}



void draw_set_fence(void)
{
	#define FENCE_ROW               (2)
	#define FENCE_COL               (6)

	lcd_clear();
	lcd_print(0, 3, "GEO FENCE", 0);

	itoa32(settings_copy.fence_threshold, &tmp_buf[0]);
	lcd_print(FENCE_ROW, FENCE_COL, &tmp_buf[0], 0);
	lcd_print_next("m", 0);

	lcd_update();
}



void draw_set_timezone(void)
{
	#define TIMEZONE_ROW               (2)
	#define TIMEZONE_COL               (4)

	lcd_clear();
	lcd_print(0, 3, "TIME ZONE", 0);

	if (settings_copy.time_zone_dir > 0)
	{
		lcd_print(TIMEZONE_ROW, TIMEZONE_COL, "+", 0);
	}
	else
	{
		lcd_print(TIMEZONE_ROW, TIMEZONE_COL, "-", 0);
	}

	itoa32(settings_copy.time_zone_hour, &tmp_buf[0]);
	add_leading_zero(&tmp_buf[0]);
	lcd_print_next(&tmp_buf[0], 0);
	lcd_print_next(":", 0);

	itoa32(settings_copy.time_zone_minute, &tmp_buf[0]);
	add_leading_zero(&tmp_buf[0]);
	lcd_print_next(&tmp_buf[0], 0);

	lcd_update();
}



void draw_confirm_settings_save(void)
{
    if (flag_settings_changed)
    {
        lcd_clear();
        lcd_print(0, 0, "Settings Changed", 0);
        lcd_print(2, 0, "OK save & reboot", 0);
        lcd_print(3, 1, "ESC cancel all", 0);
        lcd_update();
    }
    else
    {
        current_menu = M_MAIN;
        draw_current_menu();
    }
}



//--------------------------------------------------------------
//--------------------------- SET ------------------------------
//--------------------------------------------------------------

void navigation_ok(void)
{
	current_menu = M_COORDINATES;
	//draw_current_menu();
	timer4_stop(); //stop compass activity
}



//Separate subfunction; also used in save_device_ok() and delete_device_ok()
void scroll_devices_up(void)
{
    do
    {
        if (navigate_to_device == NAV_OBJECT_LAST)
        {
        	navigate_to_device = NAV_OBJECT_FIRST;
        }
        else
        {
        	navigate_to_device++;
        }
    }
    while (pp_devices[navigate_to_device]->exist_flag == 0);
}



void navigation_and_coordinates_up(void)
{
	scroll_devices_up();

    //draw_current_menu();
}



void navigation_and_coordinates_down(void)
{
    do
    {
        if (navigate_to_device == NAV_OBJECT_FIRST)
        {
        	navigate_to_device = NAV_OBJECT_LAST;
        }
        else
        {
        	navigate_to_device--;
        }
    }
    while (pp_devices[navigate_to_device]->exist_flag == 0);

    //draw_current_menu();
}



void navigation_esc(void)
{
	timer4_stop();	//stop compass activity
	current_menu = M_MAIN;
}



void coordinates_esc(void)
{
	timer4_start();	//stop compass activity
	current_menu = M_NAVIGATION;
}



void main_ok(void)
{
	switch (get_current_item())
	{
		case M_MAIN_I_NAVIGATION:
			timer4_start();					//start compass activity
			current_menu = M_NAVIGATION;
			break;
		case M_MAIN_I_DEVICES:
			current_menu = M_DEVICES;
			break;
		case M_MAIN_I_SETTINGS:
			current_menu = M_SETTINGS;
			break;
	}
}



void main_navigation_coordinates_pwr_long(void)
{
	if (current_menu == M_NAVIGATION)
	{
		timer4_stop(); //stop compass activity
	}
	return_from_power_menu = current_menu;
	current_menu = M_POWER;
	//draw_current_menu();
}



void power_ok(void)	//non standard implementation: switch the current item and do the action
{
	switch (get_current_item())
	{
		case M_POWER_I_ALARM:
			toggle_my_alarm();
			break;

		case M_POWER_I_SOUND:
			toggle_sound();
			break;

		case M_POWER_I_POWEROFF:
			release_power();
			break;

		default:
			break;
	}

	//draw_current_menu();
}



void power_esc(void)
{
	if (return_from_power_menu == M_NAVIGATION)
	{
		timer4_start(); //start compass activity
	}
	reset_current_item_in_menu(M_POWER);
	current_menu = return_from_power_menu;
	//draw_current_menu();
}



void coordinates_ok(void)
{
	current_menu = M_DEVICE_SUBMENU;
	//draw_current_menu();
}



void save_device_up(void)
{
	if (current_mem_point_to_save == MEMORY_POINT_LAST)
	{
		current_mem_point_to_save = MEMORY_POINT_FIRST;
	}
	else
	{
		current_mem_point_to_save++;
	}

	//draw_current_menu();
}



void save_device_down(void)
{
	if (current_mem_point_to_save == MEMORY_POINT_FIRST)
	{
		current_mem_point_to_save = MEMORY_POINT_LAST;
	}
	else
	{
		current_mem_point_to_save--;
	}

	//draw_current_menu();
}



void save_device_ok(void)
{
	//copy current device/point to selected mem point
	pp_devices[current_mem_point_to_save]->exist_flag = 1;
	pp_devices[current_mem_point_to_save]->latitude.as_float = pp_devices[navigate_to_device]->latitude.as_float;
	pp_devices[current_mem_point_to_save]->longitude.as_float = pp_devices[navigate_to_device]->longitude.as_float;
	pp_devices[current_mem_point_to_save]->altitude.as_integer = pp_devices[navigate_to_device]->altitude.as_integer;
	pp_devices[current_mem_point_to_save]->save_day = p_gps_num->day_tz;
	pp_devices[current_mem_point_to_save]->save_month = p_gps_num->month_tz;
	pp_devices[current_mem_point_to_save]->save_year = p_gps_num->year_tz;

	//save to flash
	store_memory_points();

	reset_current_item_in_menu(M_DEVICE_SUBMENU);
	current_mem_point_to_save = MEMORY_POINT_FIRST;		//reset current point name to save
	current_menu = M_SAVED_POPUP;
	//draw_current_menu();
}



void save_device_esc(void)
{
	current_mem_point_to_save = MEMORY_POINT_FIRST;		//reset current point name to save
	current_menu = M_DEVICE_SUBMENU;
	//draw_current_menu();
}



void delete_device_ok(void)
{
	//delete dev or mem point (except this dev)
	if (navigate_to_device != this_device)
	{
		pp_devices[navigate_to_device]->exist_flag = 0;		//del

		if (pp_devices[navigate_to_device]->memory_point_flag == 1)		//if a mem point then erase from flash too
		{
			store_memory_points();
		}
	}

	scroll_devices_up();

	reset_current_item_in_menu(M_DEVICE_SUBMENU);
	current_menu = M_DELETED_POPUP;
	//draw_current_menu();
}



void set_device_number_up(void)
{
	if (settings_copy.device_number == settings_copy.devices_on_air)
	{
		settings_copy.device_number = DEVICE_NUMBER_FIRST;

		if (settings_copy.devices_on_air == DEVICE_NUMBER_LAST)
		{
			settings_copy.devices_on_air = DEVICE_NUMBER_FIRST + 1;
		}
		else
		{
			settings_copy.devices_on_air++;
		}
	}
	else
	{
		settings_copy.device_number++;
	}

    //draw_current_menu();
}



void set_device_number_down(void)
{
	if (settings_copy.device_number == DEVICE_NUMBER_FIRST)
	{
		if (settings_copy.devices_on_air == (DEVICE_NUMBER_FIRST + 1))
		{
			settings_copy.devices_on_air = DEVICE_NUMBER_LAST;
		}
		else
		{
			settings_copy.devices_on_air--;
		}

		settings_copy.device_number = settings_copy.devices_on_air;
	}
	else
	{
		settings_copy.device_number--;
	}

    //draw_current_menu();
}



void set_device_number_ok(void)
{
    if ((settings_copy.device_number != p_settings->device_number) | (settings_copy.devices_on_air != p_settings->devices_on_air))
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_DEVICE;
    //draw_current_menu();
}



void set_device_number_esc(void)
{
    settings_copy.device_number = p_settings->device_number;   //exit no save, reset values
    settings_copy.devices_on_air = p_settings->devices_on_air;
    current_menu = M_EDIT_DEVICE;
    //draw_current_menu();
}



void set_device_id_up(void)
{
    if (settings_copy.device_id == DEVICE_ID_LAST_SYMBOL)
    {
        settings_copy.device_id = DEVICE_ID_FIRST_SYMBOL;
    }
    else
    {
        settings_copy.device_id++;
    }

    //draw_current_menu();
}



void set_device_id_down(void)
{
    if (settings_copy.device_id == DEVICE_ID_FIRST_SYMBOL)
    {
        settings_copy.device_id = DEVICE_ID_LAST_SYMBOL;
    }
    else
    {
        settings_copy.device_id--;
    }

    //draw_current_menu();
}



void set_device_id_ok(void)
{
	if (settings_copy.device_id != p_settings->device_id)
	{
		flag_settings_changed = 1;
	}

    current_menu = M_EDIT_DEVICE;
    //draw_current_menu();
}



void set_device_id_esc(void)
{
	settings_copy.device_id = p_settings->device_id;   //exit no save, reset value
    current_menu = M_EDIT_DEVICE;
    //draw_current_menu();
}



void set_freq_channel_up(void)
{
    if (settings_copy.freq_channel == FREQ_CHANNEL_LAST)
    {
        settings_copy.freq_channel = FREQ_CHANNEL_FIRST;
    }
    else
    {
        settings_copy.freq_channel++;
    }

    //draw_current_menu();
}



void set_freq_channel_down(void)
{
    if (settings_copy.freq_channel == FREQ_CHANNEL_FIRST)
    {
        settings_copy.freq_channel = FREQ_CHANNEL_LAST;
    }
    else
    {
        settings_copy.freq_channel--;
    }

    //draw_current_menu();
}



void set_freq_channel_ok(void)
{
    if (settings_copy.freq_channel != p_settings->freq_channel)
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_freq_channel_esc(void)
{
    settings_copy.freq_channel = p_settings->freq_channel;   //exit no save, reset value
    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_tx_power_up(void)
{
    if (settings_copy.tx_power_opt == TX_POWER_LAST_OPTION)
    {
        settings_copy.tx_power_opt = TX_POWER_FIRST_OPTION;
    }
    else
    {
        settings_copy.tx_power_opt++;
    }

    //draw_current_menu();
}



void set_tx_power_down(void)
{
    if (settings_copy.tx_power_opt == TX_POWER_FIRST_OPTION)
    {
        settings_copy.tx_power_opt = TX_POWER_LAST_OPTION;
    }
    else
    {
        settings_copy.tx_power_opt--;
    }

    //draw_current_menu();
}



void set_tx_power_ok(void)
{
    if (settings_copy.tx_power_opt != p_settings->tx_power_opt)
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_tx_power_esc(void)
{
    settings_copy.tx_power_opt = p_settings->tx_power_opt;   //exit no save, reset value
    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_update_interval_up(void)
{
    if (settings_copy.update_interval_opt == UPDATE_INTERVAL_LAST_OPTION)
    {
    	settings_copy.update_interval_opt = UPDATE_INTERVAL_FIRST_OPTION;
    }
    else
    {
    	settings_copy.update_interval_opt++;
    }

    //draw_current_menu();
}



void set_update_interval_down(void)
{
    if (settings_copy.update_interval_opt == UPDATE_INTERVAL_FIRST_OPTION)
    {
    	settings_copy.update_interval_opt = UPDATE_INTERVAL_LAST_OPTION;
    }
    else
    {
    	settings_copy.update_interval_opt--;
    }

    //draw_current_menu();
}



void set_update_interval_ok(void)
{
    if (settings_copy.update_interval_opt != p_settings->update_interval_opt)
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_update_interval_esc(void)
{
	settings_copy.update_interval_opt = p_settings->update_interval_opt;   //exit no save, reset value
    current_menu = M_EDIT_RADIO;
    //draw_current_menu();
}



void set_timeout_up(void)
{
	if (settings_copy.timeout_threshold >= TIMEOUT_THRESHOLD_MAX)
	{
		settings_copy.timeout_threshold = TIMEOUT_THRESHOLD_MIN;
	}
	else
	{
		settings_copy.timeout_threshold += TIMEOUT_THRESHOLD_STEP;
	}

	//draw_current_menu();
}



void set_timeout_down(void)
{
	if (settings_copy.timeout_threshold <= TIMEOUT_THRESHOLD_MIN)
	{
		settings_copy.timeout_threshold = TIMEOUT_THRESHOLD_MAX;
	}
	else
	{
		settings_copy.timeout_threshold -= TIMEOUT_THRESHOLD_STEP;
	}

	//draw_current_menu();
}



void set_timeout_ok(void)
{
    if (settings_copy.timeout_threshold != p_settings->timeout_threshold)
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_OTHER;
    //draw_current_menu();
}



void set_timeout_esc(void)
{
	settings_copy.timeout_threshold = p_settings->timeout_threshold;   //exit no save, reset value
    current_menu = M_EDIT_OTHER;
    //draw_current_menu();
}



void set_fence_up(void)
{
	if (settings_copy.fence_threshold >= FENCE_THRESHOLD_MAX)
	{
		settings_copy.fence_threshold = FENCE_THRESHOLD_MIN;
	}
	else
	{
		settings_copy.fence_threshold += FENCE_THRESHOLD_STEP;
	}

	//draw_current_menu();
}



void set_fence_down(void)
{
	if (settings_copy.fence_threshold <= FENCE_THRESHOLD_MIN)
	{
		settings_copy.fence_threshold = FENCE_THRESHOLD_MAX;
	}
	else
	{
		settings_copy.fence_threshold -= FENCE_THRESHOLD_STEP;
	}

	//draw_current_menu();
}



void set_fence_ok(void)
{
    if (settings_copy.fence_threshold != p_settings->fence_threshold)
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_OTHER;
    //draw_current_menu();
}



void set_fence_esc(void)
{
	settings_copy.fence_threshold = p_settings->fence_threshold;   //exit no save, reset value
    current_menu = M_EDIT_OTHER;
    //draw_current_menu();
}



void set_timezone_up(void)
{
	if ((settings_copy.time_zone_hour == 0) && (settings_copy.time_zone_minute == 0))
	{
		settings_copy.time_zone_dir = 1;
	}

	if (((settings_copy.time_zone_dir * settings_copy.time_zone_hour) >= 14) && (settings_copy.time_zone_minute >= 0))		//todo: make 14 as define
	{
		settings_copy.time_zone_hour = 14;
		settings_copy.time_zone_minute = 0;
	}
	else
	{
		if (settings_copy.time_zone_dir > 0)
		{
			time_zone_inc();
		}
		else
		{
			time_zone_dec();
		}
	}
}



void set_timezone_down(void)
{
	if ((settings_copy.time_zone_hour == 0) && (settings_copy.time_zone_minute == 0))
	{
		settings_copy.time_zone_dir = -1;
	}

	if (((settings_copy.time_zone_dir * settings_copy.time_zone_hour) <= -12) && (settings_copy.time_zone_minute >= 0))
	{
		settings_copy.time_zone_hour = 12;
		settings_copy.time_zone_minute = 0;
	}
	else
	{
		if (settings_copy.time_zone_dir > 0)
		{
			time_zone_dec();
		}
		else
		{
			time_zone_inc();
		}
	}
}



void time_zone_inc(void)
{
	settings_copy.time_zone_minute += 15;

	if (settings_copy.time_zone_minute > 45)
	{
		settings_copy.time_zone_minute = 0;
		settings_copy.time_zone_hour += 1;
	}
}



void time_zone_dec(void)
{
	settings_copy.time_zone_minute -= 15;

	if (settings_copy.time_zone_minute < 0)
	{
		settings_copy.time_zone_minute = 45;
		settings_copy.time_zone_hour -= 1;
	}
}



void set_timezone_ok(void)
{
    if ((settings_copy.time_zone_dir != p_settings->time_zone_dir) ||
    	(settings_copy.time_zone_hour != p_settings->time_zone_hour) ||
		(settings_copy.time_zone_minute != p_settings->time_zone_minute))
    {
        flag_settings_changed = 1;
    }

    current_menu = M_EDIT_OTHER;
}



void set_timezone_esc(void)
{
	settings_copy.time_zone_dir = p_settings->time_zone_dir;   //exit no save, reset value
	settings_copy.time_zone_hour = p_settings->time_zone_hour;
	settings_copy.time_zone_minute = p_settings->time_zone_minute;
    current_menu = M_EDIT_OTHER;
}



void confirm_settings_save_ok(void)
{
    lcd_clear();
    lcd_print(0, 3, "Saving...", 0);
    lcd_update();
    settings_save(&settings_copy);
    delay_cyc(200000);
    NVIC_SystemReset();
}



void confirm_settings_save_esc(void)
{
    settings_copy = *p_settings;   //reset to no changes state
    flag_settings_changed = 0;  //clear flag
    current_menu = M_MAIN;
    //draw_current_menu();
}



uint8_t get_current_device(void)
{
	return navigate_to_device;
}
