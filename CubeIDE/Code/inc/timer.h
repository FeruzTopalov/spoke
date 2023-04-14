/*
	SPOKE
    
    file: timer.h
*/



#ifndef TIMER_HEADER
#define TIMER_HEADER



void timers_init(void);
void timer1_start(void);
void timer1_stop_reload(void);
void timer2_start(void);
void timer2_stop(void);
void toggle_sound(void);
uint8_t get_sound_status(void);



#endif /*TIMER_HEADER*/
