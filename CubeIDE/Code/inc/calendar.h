/*
	SPOKE

    file: calendar.h
*/



#ifndef CALENDAR_HEADER
#define CALENDAR_HEADER



#define CALENDAR_START_YEAR (22)

const int8_t calendar[][12] =
{
	    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, 	//2022
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	//2023
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	//2024
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	//2025
		//todo: extend
};



#endif /*CALENDAR_HEADER*/
