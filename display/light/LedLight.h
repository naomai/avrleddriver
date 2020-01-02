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

class LedLight{
	friend class LedDriver;
	protected:
	light_s *stateHW;
	lightType type; // type of light (RGB, white, etc)
	colorRaw userColor; // color selected by user
	colorRaw tempColor; // currently displayed color (changed by menu, animations, etc)
	uint8_t myId;
	ColorMapper *mapper;
	
	public:
	uint8_t special; // flags (animation, random color, etc)
	LedLight(light_s *state);

	
	void setColor(colorRaw localColor, lightColorType which);
	colorRaw getColor(lightColorType which);
	void applySpecialColor();
	void setSpecialAttribute(uint8_t special);
	void resetTempColor();
	
};

#include "../../LedDriver.h"


#endif /* LEDSTRIP_H_ */