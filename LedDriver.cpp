/*
 * LedDriver.cpp
 *
 * Created: 27.12.2019 12:22:55
 *  Author: bun
 */ 


#include "LedDriver.h"
#include "display/animation.h"
#include "hardware/pwm/pwm.h"

extern const lightConfig PROGMEM lightsConfig[STRIPS_COUNT] = STRIPS_CONFIG;;

LedDriver::LedDriver() : Module(){
	lightConfig cfg;
	light_s *stateHW;
	LedLight * s;
	this->ledHal = new LedHardware();
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		memcpy_P(&cfg, &lightsConfig[i], sizeof(lightConfig));
		stateHW = ledHal->registerLight(&cfg);
		s = new LedLight(stateHW);
		s->myId = i;
		
		// fill with special random color
		s->setSpecialAttribute(P_SPECIAL_RANDOM>>24);
		s->applySpecialColor();
		this->lights[i] = s;
	}

}

LedDriver::~LedDriver(){
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		delete this->lights[i];
	}
	delete this->ledHal;
}

void LedDriver::lateInit(){
	fadeInSequence();
}

void LedDriver::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	uint16_t frameId = (hbyte<<8) | lbyte;
	if(type==EVENT_FRAME){
		if(frameId % 16 == 0){
			for(uint8_t i=0; i<STRIPS_COUNT; i++){
				LedLight *l;
				l = this->getLightById(i);
				l->resetTempColor();
			}
		}
		
		this->ledHal->tick();
	}
}

LedLight* LedDriver::getLightById(uint8_t lightId){
	return this->lights[lightId];
}

void LedDriver::fadeInSequence(){
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		animation * anim;
		anim = animCreate(i, colorBlack, this->getLightById(i)->getColor(LIGHT_COLOR_USER), 16 << 8, (animType)ANIM_INTROCHAIN);
		if(i==0){
			animStart(anim);
		}
	}
}