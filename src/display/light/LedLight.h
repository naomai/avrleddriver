/*
 * LedStrip.h
 *
 * Created: 22.12.2019 23:15:06
 *  Author: bun
 */ 


#ifndef LEDSTRIP_H_
#define LEDSTRIP_H_

#include <stdbool.h>
#include "ColorMapper.h"
#include "../color.h"
#include "../../hardware/LedHardware.h"
#include "../../PowerMgmt.h"

typedef enum {
	COLORSPACE_SRGB = 0x00,
	COLORSPACE_RAW = 0x01	
}colorSpace;

class LedLight{
	friend class LedDriver;
	protected:
	light_s *stateHW;
	lightType type; // type of light (RGB, white, etc)
	colorRaw memColor; // color selected by user (in memory)
	colorRaw userColor; // color selected by user (temporary)
	colorRaw tempColor; // color in current frame (modified by menu, animations, etc)
	uint8_t myId;
	
	public:
	uint8_t special; // flags (animation, random color, etc)
	LedLight(light_s *state);
	ColorMapper *mapper;
	
	
	void setColor(colorRaw color, lightColorType which, colorSpace space = COLORSPACE_SRGB);
	colorRaw getColor(lightColorType which, colorSpace space = COLORSPACE_SRGB);
	colorRaw maskColor(colorRaw color);
	void applySpecialColor();
	void setSpecialAttribute(uint8_t special);
	void resetTempColor();
	void setPowerState(powerState pwr);
	
};

#include "../../LedDriver.h"


#endif /* LEDSTRIP_H_ */