/*
    SPOKE

    file: config.h
*/



#ifndef CONFIG_HEADER
#define CONFIG_HEADER



/* PART A - user-defined */

/* Step 1 - Select Spoke HW Revision */
//#define SPOKE_HW_REV_1_x	/* Spoke HW 1.x */
//#define SPOKE_HW_REV_2_0	/* Spoke HW 2.0 */
#define SPOKE_HW_REV_2_1	/* Spoke HW 2.1 */

/* Step 2 - Select GPS Baud Speed */
//#define GPS_BAUD_9600
#define GPS_BAUD_38400
//#define GPS_BAUD_57600
//#define GPS_BAUD_115200	/* Operation at 115200 is not guaranteed due to limited buffer size */

/* Step 3 - Select Frequency Band */
//#define FREQ_BAND_433		/* Define for E22-400M22S module (SX1268) if it is populated in any HW revision */
#define FREQ_BAND_868		/* Define for E22-900M22S module (SX1262) if it is populated in any HW revision */



/* PART B - auto-defined (change only if you are sure what you do) */

/* Step 1 - Define/undefine power hold line fix, either legacy or swapping POWER_HOLD (PA15) and RF_TXEN (PA11) */
#ifdef SPOKE_HW_REV_2_1
#define POWER_HOLD_FIX		/* Undefine for HW 1.x or 2.0; Define for HW 2.1 and above */
#endif

/* Step 2 - Define LCD type, either OLED SH1106 or Monochrome with BL ST7567A*/
#ifdef SPOKE_HW_REV_1_x
#define LCD_TYPE_SH1106		/* Define for HW 1.x */
#endif

#if defined(SPOKE_HW_REV_2_0) || defined(SPOKE_HW_REV_2_1)
#define LCD_TYPE_ST7567A	/* Define for HW 2.x */
#endif

/* Step 3 - Define/undefine passive buzzer drive type and LCD backlight control type, either differential buzzer drive and on/off backlight (legacy) or single-ended buzzer drive and PWM backlight (new) */
#ifdef SPOKE_HW_REV_2_1
#define SPLIT_PWM_BUZZER_BACKLIGHT		/* Undefine for HW 1.x or 2.0; Define for HW 2.1 and above */
#endif



#endif /*CONFIG_HEADER*/
