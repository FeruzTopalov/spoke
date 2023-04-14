/*
	SPOKE
    
    file: buttons.h
*/



#ifndef BUTTONS_HEADER
#define BUTTONS_HEADER



#define BUTTON_PRESSED_COUNTER_THRESHOLD    (55)       	//short or long cick bounadry


//Button physical pins 		(pin number)
#define BUTTON_UP_OK_PA1	(1)
#define BUTTON_DOWN_ESC_PA2	(2)
#define BUTTON_PWR_PA3		(3)


//Button actions
#define BTN_NO_ACTION		(0)

#define BTN_UP				(1)     //PA1
#define BTN_OK				(2)

#define BTN_DOWN			(3)     //PA2
#define BTN_ESC				(4)

#define BTN_PWR				(5)     //PA3
#define BTN_PWR_LONG		(6)



uint8_t scan_button(uint8_t button);



#endif /*BUTTONS_HEADER*/
