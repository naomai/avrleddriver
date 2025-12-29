/*
 * LedDriver.cpp
 *
 * Created: 27.12.2019 12:22:55
 *  Author: bun
 */ 

#include <stddef.h>
#include "LedDriver.h"
#include "display/animation.h"
#include "hardware/pwm/pwm.h"
#include "PowerMgmt.h"

extern const entityConfig PROGMEM lightsConfig[ENTITY_COUNT] = LIGHTS_CONFIG;
extern const lightCalibration PROGMEM calibrationTable[] = CALIBRATION_TABLE;

LedDriver::LedDriver() : Module(){
	entityConfig cfg;
	light_s *stateHW;
	LedLight * s;
	this->ledHal = new LedHardware();
	for(uint8_t i=0; i<ENTITY_COUNT; i++){
		memcpy_P(&cfg, &lightsConfig[i], sizeof(entityConfig));
		if((cfg.type & 0xE0) == SENSOR_MASK) {
			continue;
		}
		stateHW = ledHal->registerLight(&cfg);
		s = new LedLight(stateHW);
		s->myId = i;
		
		// fill with special random color
		// if using Settings module, it will be overriden
		s->setSpecialAttribute(P_SPECIAL_RANDOM);
		s->applySpecialColor();
		this->lights[i] = s;
	}
	this->ledHal->initFinished();
	powerDown = false;

}

LedDriver::~LedDriver(){
	for(uint8_t i=0; i<ENTITY_COUNT; i++){
		if(this->lights[i]==NULL) {
			continue;
		}
		delete this->lights[i];
	}
	delete this->ledHal;
}

void LedDriver::lateInit(){
	//fadeInSequence(); // handled by EVENT_POWERSTATE
}

void LedDriver::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	uint16_t frameId = (hbyte<<8) | lbyte;
	if(type==EVENT_FRAME){
		if(!powerDown && frameId % 16 == 0){
			for(uint8_t i=0; i<ENTITY_COUNT; i++){
				LedLight *l;
				l = this->getLightById(i);
				if(l!=NULL) {
					l->resetTempColor();
				}
			}
		}
		
		this->ledHal->tick();
	}else if(type==EVENT_DEEP_SLEEP_REQUESTED){
		setPowerStateForAll(PS_OFF);
	}else if(type==EVENT_POWERSTATE){
		if(lbyte == 255){
			powerDown = ((powerState)hbyte == PS_OFF) ;
			setPowerStateForAll((powerState)hbyte);
		}else{
			LedLight *l = this->getLightById(lbyte);
			if(l != NULL){
				l->setPowerState((powerState)hbyte);
			}
		}
	}
}

LedLight* LedDriver::getLightById(uint8_t lightId){
	if(lightId >= ENTITY_COUNT) return NULL;
	return this->lights[lightId];
}

void LedDriver::setPowerStateForAll(powerState pwr){
	for(uint8_t i=0; i<ENTITY_COUNT; i++){
		LedLight *l = this->getLightById(i);
		if(l!=NULL){
			l->setPowerState(pwr);
		}
	}	
}

