/*
 * LedDriver.h
 *
 * Created: 27.12.2019 20:27:08
 *  Author: bun
 */ 


#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "hardware/LedHardware.h"
#include "display/light/LedLight.h"
#include "modular/Module.h"

class LedDriver : public Module{
	protected:
	LedLight *lights[STRIPS_COUNT];
	LedHardware *ledHal;
	
	public:
	LedDriver();
	~LedDriver();
	
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void lateInit();
	LedLight* getLightById(uint8_t lightId);
	void fadeInSequence();
};

#endif /* LEDDRIVER_H_ */