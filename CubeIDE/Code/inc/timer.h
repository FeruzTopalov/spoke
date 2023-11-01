/*
	SPOKE
    
    file: timer.h
*/



#ifndef TIMER_HEADER
#define TIMER_HEADER


void timers_init(void);
void make_a_beep(void);
void make_a_long_beep(void);
void rtc_init(void);
void systick_stop(void);
void timer1_start(void);
void timer1_stop_reload(void);
void timer2_stop(void);
void timer3_start(void);
void timer3_stop(void);
void timer4_start(void);
void timer4_stop(void);
void toggle_sound(void);
uint8_t get_sound_status(void);



#endif /*TIMER_HEADER*/
