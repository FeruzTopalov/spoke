/*
    SPOKE
    
    file: menu.h
*/



#ifndef MENU_HEADER
#define MENU_HEADER



void init_menu(void);
void change_menu(uint8_t button_code);
void draw_current_menu(void);
uint8_t get_current_device(void);



#endif /*MENU_HEADER*/
