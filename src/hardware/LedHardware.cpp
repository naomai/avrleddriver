/*
 * LedHardware.cpp
 *
 * Created: 22.12.2019 21:43:08
 *  Author: bun
 */ 

#include "LedHardware.h"
#include "../display/color.h"
#include "pwm/pwm.h"



LedHardware::LedHardware(){
	lights = new LinkedList();
	initPwm();
}
LedHardware::~LedHardware(){
	for(light_s *s=(light_s*)lights->reset(); lights->isValid(); s=(light_s*)lights->next()){
		delete s;
	}
	delete lights;	
}
	
light_s* LedHardware::registerLight(lightConfig *config){
	light_s *s = new light_s;
	s->hardwareConfig = *config;
	s->color = colorWhite;
	lights->add(s);
	return s;
}
	
void LedHardware::tick(){
	if(pwmIsReady()){ // else drop frame
		pwmPrepareStart();
		//generate led turnoff time table
		for(light_s *s=(light_s*)lights->reset(); lights->isValid(); s=(light_s*)lights->next()){
			pwmBufferLight(s);
		}
		pwmPrepareFinish();
	}
}


