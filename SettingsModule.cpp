/*
 * SettingsModule.cpp
 *
 * Created: 27.12.2019 21:30:09
 *  Author: bun
 */ 

#include "SettingsModule.h"
#include "control/Menu.h"
#include "settings.h"
#include "display/color.h"
#include "display/animation.h"
#include "LedDriver.h"

extern LedDriver *lights;

Settings::Settings() : Module() {
	updateRequested=false;
	this->tickPrescaler = 200;
}

void Settings::lateInit(){
	if(!loadSettings()){ // no settings in memory, load defaults
		//setting special byte for random color is moved to LedDriver::LedDriver() 
		updateSettings();
	}
}

void Settings::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	//if(type==EVENT_MENU_SELECT && lbyte == MENU_NOTHING){
	if(type==EVENT_COLOR_CHANGE && hbyte == LIGHT_COLOR_SET){
		updateRequested = true;
		
	}
}

void Settings::tick(){
	if(updateRequested){
		updateRequested=false;
		updateSettings();
		this->saverBlinker();
	}
}

void Settings::saverBlinker(){
	animation * anim;
	LedLight * l;
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		l = lights->getLightById(i);
		anim = animCreate(i, colorWhite, l->getColor(LIGHT_COLOR_USER), 8 << 8, ANIM_TEMPFX);
		animStart(anim);
	}
}