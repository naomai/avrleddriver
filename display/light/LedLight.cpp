/*
 * strip.c
 *
 * Created: 22.12.2019 20:32:25
 *  Author: bun
 */ 
#include "../../hardware/LedHardware.h"
#include "../../types.h"
#include "LedLight.h"
#include "../../modular/Dispatcher.h"

extern Dispatcher *dispatcher;


LedLight::LedLight(light_s *state) : stateHW(state), type(state->hardwareConfig.type) {
	this->mapper = ColorMapper::createMapper(&state->hardwareConfig);
}

void LedLight::setColor(colorRaw localColor, lightColorType which){
	if(which==LIGHT_COLOR_DISPLAY){
		this->tempColor = localColor;
		this->stateHW->color = this->mapper->fromRGB(localColor);
		//this->stateHW->color = localColor;
	}else if(which == LIGHT_COLOR_USER || which == LIGHT_COLOR_SET){
		this->userColor = localColor;
	}else{
		return;
	}
	
	eventDescriptor ev;
	ev.type = EVENT_COLOR_CHANGE;
	ev.lbyte = this->myId;
	ev.hbyte = which;
	dispatcher->queue->pushEvent(ev);
}

colorRaw LedLight::getColor(lightColorType which){
	if(which==LIGHT_COLOR_DISPLAY){
		return this->tempColor;
	}else{
		return this->userColor;
	}
}

void LedLight::applySpecialColor(){
	if(this->special & (P_SPECIAL_RANDOM>>24)){
		if(this->special & 0xF0){ // P_SPECIAL_DONTRESET
			this->setColor(HSV2RGB(randomColor()),LIGHT_COLOR_DISPLAY);
		}else{
			this->setColor(HSV2RGB(randomColor()),LIGHT_COLOR_USER);
		}
	}
}
void LedLight::setSpecialAttribute(uint8_t special){
	this->special &= 0xF0;
	this->special |= special & 0x0F;
	this->applySpecialColor();
}

void LedLight::resetTempColor(){
	//this->tempColor = this->userColor;
	if((this->special & 0xF0)==0){ // P_SPECIAL_DONTRESET
		this->setColor(this->userColor, LIGHT_COLOR_DISPLAY);
	}
}

