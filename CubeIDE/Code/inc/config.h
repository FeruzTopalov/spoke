/*
    SPOKE

    file: config.h
*/



#ifndef CONFIG_HEADER
#define CONFIG_HEADER



#define DEVICES_ON_AIR_MAX	(5)												//max number of devices on air (real Spoke devices)
#define MEMORY_POINTS_TOT	(4)												//total number of memory points
#define NAV_OBJECTS_MAX		(DEVICES_ON_AIR_MAX + MEMORY_POINTS_TOT)         //max number of devices including real Spoke devices and memory points

#define DEVICE_NUMBER_FIRST (1)
#define DEVICE_NUMBER_LAST  (DEVICES_ON_AIR_MAX)

#define MEMORY_POINT_FIRST	(DEVICES_ON_AIR_MAX + 1)
#define MEMORY_POINT_LAST	(MEMORY_POINT_FIRST + MEMORY_POINTS_TOT - 1)

#define NAV_OBJECT_FIRST 	(1)
#define NAV_OBJECT_LAST  	(NAV_OBJECTS_MAX)



#endif /*CONFIG_HEADER*/
