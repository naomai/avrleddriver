/*
 * LightConfig.h
 *
 * Created: 26.12.2019 18:56:33
 *  Author: bun
 */ 


#ifndef LIGHTCONFIG_H_
#define LIGHTCONFIG_H_

#include "color.h"

#define MAX_BYTES_PER_ENTITY 5 // RGB + WW (CCT)

typedef enum{
	LIGHT_RED = 0x04,
	LIGHT_GREEN = 0x02,
	LIGHT_BLUE = 0x01,
	LIGHT_WHITE = 0x08,
	LIGHT_WHITEWARM = 0x10,
	LIGHT_CCT = LIGHT_WHITE|LIGHT_WHITEWARM,
	LIGHT_RGB = LIGHT_RED|LIGHT_GREEN|LIGHT_BLUE,
	LIGHT_RGBW = LIGHT_RGB|LIGHT_WHITE,
	LIGHT_RGBCCT = LIGHT_RGB|LIGHT_CCT,
	// UGLY
	SENSOR_MASK = 0x20,
	SENSOR_HUMIDITY = SENSOR_MASK | 0x01,
	SENSOR_TEMPERATURE = SENSOR_MASK | 0x02,
	SENSOR_BATTERY = SENSOR_MASK | 0x03,
} entityType;

typedef enum{
	LIGHT_COLOR_SET, // memory
	LIGHT_COLOR_USER, // temporary
	LIGHT_COLOR_DISPLAY	// current frame
} lightColorType;


typedef struct {
	//dim factor
	uint8_t dr; 
	uint8_t dg;
	uint8_t db;
	uint8_t dc1;
	uint8_t dc2;
	//colors of additional components
	colorRGB add1;
	colorRGB add2;
}lightCalibration;

typedef struct {
	entityType type;
	uint8_t pins[MAX_BYTES_PER_ENTITY];
	uint8_t calibrationIdx;		
} entityConfig;



#endif /* LIGHTCONFIG_H_ */