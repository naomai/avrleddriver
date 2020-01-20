/*
 * LightConfig.h
 *
 * Created: 26.12.2019 18:56:33
 *  Author: bun
 */ 


#ifndef LIGHTCONFIG_H_
#define LIGHTCONFIG_H_

#include "color.h"

#define MAX_CHANNELS_PER_LIGHT 5 // RGB + WW (CCT)

typedef enum{
	LIGHT_RED = 0x04,
	LIGHT_GREEN = 0x02,
	LIGHT_BLUE = 0x01,
	LIGHT_WHITE = 0x08,
	LIGHT_WHITEWARM = 0x10,
	LIGHT_CCT = LIGHT_WHITE|LIGHT_WHITEWARM,
	LIGHT_RGB = LIGHT_RED|LIGHT_GREEN|LIGHT_BLUE,
	LIGHT_RGBW = LIGHT_RGB|LIGHT_WHITE,
	LIGHT_RGBCCT = LIGHT_RGB|LIGHT_CCT
} lightType;

typedef enum{
	LIGHT_COLOR_SET, LIGHT_COLOR_USER, LIGHT_COLOR_DISPLAY	
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
	lightType type;
	uint8_t pins[MAX_CHANNELS_PER_LIGHT];
	uint8_t calibrationIdx;		
} lightConfig;



#endif /* LIGHTCONFIG_H_ */