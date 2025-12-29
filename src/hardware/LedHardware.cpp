/*
 * LedHardware.cpp
 *
 * Created: 22.12.2019 21:43:08
 *  Author: bun
 */ 

#include "LedHardware.h"
#include "../display/color.h"
#include "pwm/pwm.h"
#include <string.h>


LedHardware::LedHardware(){
	light_list = new LinkedList();
	PWMChannels = 0;
}
LedHardware::~LedHardware(){
	for(light_s *s=(light_s*)light_list->reset(); light_list->isValid(); s=(light_s*)light_list->next()){
		delete s;
	}
	delete light_list;	
}
	
light_s* LedHardware::registerLight(entityConfig *config){
	light_s *s = new light_s;
	memcpy(&s->hardwareConfig, config, sizeof(entityConfig));
	s->color = colorWhite;
	light_list->add(s);
	
	if(config->type & LIGHT_BLUE) PWMChannels++;
	if(config->type & LIGHT_GREEN) PWMChannels++;
	if(config->type & LIGHT_RED) PWMChannels++;
	if(config->type & LIGHT_WHITE) PWMChannels++;
	if(config->type & LIGHT_WHITEWARM) PWMChannels++;
	return s;
}

void LedHardware::initFinished(){
	initPwm();
}
	
void LedHardware::tick(){
	if(pwmIsReady()){ // else drop frame
		pwmPrepareStart();
		//generate led turnoff time table
		for(light_s *s=(light_s*)light_list->reset(); light_list->isValid(); s=(light_s*)light_list->next()){
			pwmBufferLight(s);
		}
		pwmPrepareFinish();
	}
}


