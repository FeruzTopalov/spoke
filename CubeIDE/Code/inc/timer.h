/*
	SPOKE
    
    file: timer.h
*/



#ifndef TIMER_HEADER
#define TIMER_HEADER



#define TIMER1_INTERVAL_TYPE_LONG		(1)
#define TIMER1_INTERVAL_TYPE_SHORT		(2)



void timers_init(void);
void make_a_beep(void);
void make_a_long_beep(void);
void init_watchdog(void);
void reload_watchdog(void);
void rtc_init(void);
void systick_stop(void);
void timer1_start_800ms(void);
void timer1_start_100ms(void);
void timer1_stop_reload(void);
uint8_t timer1_get_intrvl_type(void);
void buzzer_pwm_stop(void);
void backlight_pwm_set(uint8_t pwm_level);
void timer3_start(void);
void timer3_stop(void);
void timer4_start(void);
void timer4_stop(void);
void toggle_sound(void);
uint8_t get_sound_status(void);



#define BL_PWM_LEVEL_OFF   		(0)
#define BL_PWM_LEVEL_LOW		(1)
#define BL_PWM_LEVEL_MID		(2)
#define BL_PWM_LEVEL_MAX		(3)



#endif /*TIMER_HEADER*/
