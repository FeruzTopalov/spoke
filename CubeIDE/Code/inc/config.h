/*
    SPOKE

    file: config.h
*/



#ifndef CONFIG_HEADER
#define CONFIG_HEADER


/* STEP 1 */
/* Select GPS Baud speed */
//#define GPS_BAUD_9600
#define GPS_BAUD_38400
//#define GPS_BAUD_57600
//#define GPS_BAUD_115200	/* Operation is not guaranteed due to limited buffer size */



/* STEP 2 */
/* Define/undefine power hold line fix, either legacy or swapping POWER_HOLD (PA15) and RF_TXEN (PA11) */
#define POWER_HOLD_FIX		/* Undefine for HW 1.x or 2.0; Define for HW 2.1 and above */



/* STEP 3 */
/* Define LCD type, either OLED SH1106 or Monochrome with backlight ST7567A */
//#define LCD_TYPE_SH1106		/* Define for HW 1.x */
#define LCD_TYPE_ST7567A	/* Define for HW 2.x */



/* STEP 4 */
/* Define/undefine passive buzzer drive type and LCD backlight control type, either differential buzzer drive and on/off backlight (legacy) or single-ended buzzer drive and PWM backlight (new) */
#define SPLIT_PWM_BUZZER_BACKLIGHT		/* Undefine for HW 1.x or 2.0; Define for HW 2.1 and above */



/* STEP 5 */
/* Define Frequency Band */
//#define FREQ_BAND_433		/* Define for E22-400M22S module (SX1268) for any HW revision */
#define FREQ_BAND_868		/* Define for E22-900M22S module (SX1262) for any HW revision */



#endif /*CONFIG_HEADER*/
