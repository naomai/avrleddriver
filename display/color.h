/*
 * color.h
 *
 *  Created on: 10 paü 2019
 *      Author: bun
 */

#ifndef COLOR_H_
#define COLOR_H_

#include "../mcu.h"
#include <avr/pgmspace.h>

typedef struct{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	//uint8_t special;
} colorRGB;

typedef union{
	uint32_t rgbLong;
	
	struct {
		uint8_t _unused0;
		uint8_t _unused1;
		uint8_t _unused2;
		//uint8_t special;
		uint8_t w;
	} white;
	
	struct {
		uint8_t _unused0;
		uint8_t _unused1;
		uint8_t _unused2;
		//uint8_t special;
		uint8_t cold;
		uint8_t warm;
	} cct;
	
	colorRGB rgb;
} colorRaw;

typedef struct{
	int16_t b;
	int16_t g;
	int16_t r;
	int16_t cold;
	int16_t warm;
}colorRaw16;

typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} colorHSV;

#ifdef __cplusplus
extern "C" {
#endif

colorRaw HEX2RGB(uint32_t color);
colorHSV RGB2HSV(colorRaw rgb);
colorRaw HSV2RGB(colorHSV hsv);

colorRaw blendColor(colorRaw c1, colorRaw c2, uint8_t alpha);
colorHSV randomColor();

uint8_t getLogBrightnessForValue(uint8_t value);
uint8_t compareColors(colorRaw source, colorRaw filter);

extern const colorRaw colorBlack;
extern const colorRaw colorWhite;
//extern const uint16_t PROGMEM linearBrightnessLut[256];

#ifdef __cplusplus
}
#endif

#endif /* COLOR_H_ */
