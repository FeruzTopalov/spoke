/*
    SPOKE
    
    file: gps.h
*/



#ifndef GPS_HEADER
#define GPS_HEADER



#define GPS_DATA_INVALID    (0)
#define GPS_DATA_VALID      (1)
#define GPS_POSITION_NOFIX  (1)
#define GPS_POSITION_2DFIX  (2)
#define GPS_POSITION_3DFIX  (3)



//GPS data from GNSS module (raw)
#define CHAR_RESERVE        (2)
struct gps_raw_struct
{
    char time[sizeof("hhmmss.ss") + CHAR_RESERVE];          //UTC Time
    char date[sizeof("ddmmyy") + CHAR_RESERVE];             //UTC Date
    char latitude[sizeof("ddmm.mmmm") + CHAR_RESERVE];      //Shirota
    char ns[sizeof("x") + CHAR_RESERVE];                    //Northing
    char longitude[sizeof("dddmm.mmmm") + CHAR_RESERVE];    //Dolgota
    char ew[sizeof("x") + CHAR_RESERVE];                    //Easting
    char speed[sizeof("xxx.yyy") + CHAR_RESERVE];           //Speed over ground, knots
    char course[sizeof("xxx.y") + CHAR_RESERVE];            //Course over ground, deg
    char status[sizeof("x") + CHAR_RESERVE];                //A=Valid, V=Invalid
    char altitude[sizeof("aaaaa.a") + CHAR_RESERVE];        //Altitude above means sea level, meters probably
    char sat_view[sizeof("nn") + CHAR_RESERVE];             //Number of SVs used for navigation
    char sat_used[sizeof("nn") + CHAR_RESERVE];             //Number of SVs used for navigation
    char mode[sizeof("x") + CHAR_RESERVE];                  //0=No, 1=2D, 2=3D
    char pdop[sizeof("xx.yy") + CHAR_RESERVE];              //Positional dilution of precision
};



//Similar to (but not the same as) gps_raw_struct, in numerical format
struct gps_num_struct
{
    uint8_t hour;       //UTC Time
    uint8_t minute;
    uint8_t second;

    uint8_t day;        //UTC Date
    uint8_t month;
    uint8_t year;

    uint8_t hour_tz;       //Time in selected time zone
    uint8_t minute_tz;

    uint8_t day_tz;        //Date in selected time zone
    uint8_t month_tz;
    uint8_t year_tz;

    float latitude;   	//Lat in decimal deg
    float longitude;   	//Lon in decimal deg

    float speed;        //kilometers per hour (converted)
    float course;       //decimal degrees
    float altitude;     //meters above means sea level, signed

    uint8_t sat_view;
    uint8_t sat_used;

    uint8_t status;     //0=Invalid, 1=Valid
    uint8_t mode;       //1=NoFix, 2=2D, 3=3D
    float pdop;
};



void gps_init(void);
uint8_t parse_gps(void);
char *get_nmea_buf(void);
struct gps_raw_struct *get_gps_raw(void);
struct gps_num_struct *get_gps_num(void);



#endif /*GPS_HEADER*/
