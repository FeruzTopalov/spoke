/*
	SPOKE
    
    file: buttons.c
*/



#include "stm32f10x.h"
#include "buttons.h"
#include "gpio.h"
#include "gpio.h"
#include "timer.h"



#define BUTTONS_NUM             (3)     //three buttons total

#define BUTTON_PRESSED          (0)
#define BUTTON_RELEASED         (1)

#define BUTTON_PRESSED_COUNTER_MIN          (5)			//minimum counts to pass
#define BUTTON_PRESSED_COUNTER_MAX          (UINT8_MAX)

#define TIMEOUT_NO_OVERFLOW     (0)
#define TIMEOUT_OVERFLOW        (1)

#define BUTTON_ACTIONS_NUM      (2)     //two actions: short click, long click
#define BUTTON_ACTION_SHORT     (0)
#define BUTTON_ACTION_LONG      (1)



uint32_t idr_reg = 0;
uint8_t button_state = BUTTON_RELEASED;
uint8_t button_prev_state = BUTTON_RELEASED;
uint8_t button_pressed_counter = 0;
uint8_t timeout_state = TIMEOUT_NO_OVERFLOW;
uint8_t timeout_prev_state = TIMEOUT_NO_OVERFLOW;



//button return code = {ButtonNumber(0...BUTTONS_NUM-1) * BUTTON_ACTIONS_NUM + BUTTON_ACTION(_SHORT)(_LONG)} + 1
uint8_t scan_button(uint8_t button)
{
	idr_reg = GPIOB->IDR;           //save pins data

	idr_reg >>= button;       		//align bits to the right, button of interest in LSBit position
	idr_reg &= (uint32_t)0x01;		//unmask needed pin bit

	button -= BUTTON_DOWN_ESC_PB3;		//subtract first button pin number to make first button index equal 0



	if (idr_reg) //check pin state
	{
		button_prev_state = button_state;	// if input pin = 1
		button_state = BUTTON_RELEASED;
	}
	else
	{
		button_prev_state = button_state;	// if input pin = 0
		button_state = BUTTON_PRESSED;
	}



	if ((button_state == BUTTON_PRESSED))   //increase timeout counter, update overflow states
	{
		if (button_pressed_counter < BUTTON_PRESSED_COUNTER_MAX)
		{
			button_pressed_counter++;
			timeout_prev_state = timeout_state;
			timeout_state = (button_pressed_counter > BUTTON_PRESSED_COUNTER_THRESHOLD);     //overflow or not? 1 or 0?
		}
		else
		{
			return BTN_NO_ACTION; //overflow max
		}
	}



	if (button_state == BUTTON_RELEASED)
	{
		if (button_prev_state == BUTTON_PRESSED)	//Button has been released
		{
			if (button_pressed_counter < BUTTON_PRESSED_COUNTER_MIN)	//lower than min, skip this
			{
				button_pressed_counter = 0;
				timer3_stop();
				enable_buttons_interrupts();
				return BTN_NO_ACTION;
			}
			else if (button_pressed_counter < BUTTON_PRESSED_COUNTER_THRESHOLD)	//released before overflow, then short click
			{
				button_pressed_counter = 0;
				timer3_stop();
				enable_buttons_interrupts();
				return (button * BUTTON_ACTIONS_NUM + BUTTON_ACTION_SHORT) + 1;
			}
			else	//released after overflow or timeout, just switch timer off with no action
			{
				button_pressed_counter = 0;
				timer3_stop();
				enable_buttons_interrupts();
				return BTN_NO_ACTION;
			}
		}
		else	//two "released" states in a row = fast button release right after interrupt, skip this
		{
			timer3_stop();
			enable_buttons_interrupts();
			return BTN_NO_ACTION;
		}
	}



	if ((timeout_state == TIMEOUT_OVERFLOW) && (timeout_prev_state == TIMEOUT_NO_OVERFLOW))   //overflow occured
	{
		return (button * BUTTON_ACTIONS_NUM + BUTTON_ACTION_LONG) + 1;   //long click
	}



	return BTN_NO_ACTION; //nothing happend, for ex. counting up while button is pressed
}
