/*
    SPOKE

    file: config.h
*/



#ifndef CONFIG_HEADER
#define CONFIG_HEADER


/* STEP 1 */
/* Select GPS Baud speed */
#define GPS_BAUD_9600
//#define GPS_BAUD_38400
//#define GPS_BAUD_57600
//#define GPS_BAUD_115200	/* Operation is not guaranteed due to limited buffer size */

/* STEP 2 */
/* Define/undefine POWER_HOLD fix */
#define POWER_HOLD_FIX	/* Undefine for HW 1.x or 2.0; Define for HW 2.1 and above */



#endif /*CONFIG_HEADER*/
