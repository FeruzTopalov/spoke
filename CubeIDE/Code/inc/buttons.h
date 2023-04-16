/*
	SPOKE
    
    file: buttons.h
*/



#ifndef BUTTONS_HEADER
#define BUTTONS_HEADER



#define BUTTON_PRESSED_COUNTER_THRESHOLD    (55)       	//short or long click boundary


//Button physical pins 		(pin number)
#define BUTTON_DOWN_ESC_PB3	(3)
#define BUTTON_UP_OK_PB4	(4)
#define BUTTON_PWR_PB5		(5)


//Button actions
#define BTN_NO_ACTION		(0)

#define BTN_UP				(1)     //PB4
#define BTN_OK				(2)

#define BTN_DOWN			(3)     //PB3
#define BTN_ESC				(4)

#define BTN_PWR				(5)     //PB5
#define BTN_PWR_LONG		(6)



uint8_t scan_button(uint8_t button);



#endif /*BUTTONS_HEADER*/
