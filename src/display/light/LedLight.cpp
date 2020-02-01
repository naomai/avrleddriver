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
#include "../animation.h"


extern Dispatcher *dispatcher;

#pragma GCC optimize("Os")


LedLight::LedLight(light_s *state) : stateHW(state), type(state->hardwareConfig.type) {
	this->mapper = ColorMapper::createMapper(&state->hardwareConfig);
	//setPowerState(PS_OFF);
	this->special |= P_SPECIAL_POWERDOWN;
	this->tempColor = colorBlack;
}

void LedLight::setColor(colorRaw color, lightColorType which, colorSpace space){
	colorRaw localColor;
	if(space==COLORSPACE_SRGB){
		localColor = this->mapper->fromRGB(color);
	}else{
		localColor = color;
	}
	localColor = maskColor(localColor);

	if(which==LIGHT_COLOR_DISPLAY){
		this->tempColor = localColor;
		this->stateHW->color = localColor;
	}else if(which == LIGHT_COLOR_USER){
		this->userColor = localColor;
	}else if(which == LIGHT_COLOR_SET){
		this->memColor = this->userColor = localColor;
	}else{
		return;
	}
	
	eventDescriptor ev;
	ev.type = EVENT_COLOR_CHANGE;
	ev.lbyte = this->myId;
	ev.hbyte = which;
	dispatcher->queue->pushEvent(ev);
}

colorRaw LedLight::getColor(lightColorType which, colorSpace space){
	colorRaw result;
	if(which==LIGHT_COLOR_DISPLAY){
		result = this->tempColor;
	}else if(which==LIGHT_COLOR_USER){
		result = this->userColor;
	}else{
		result = this->memColor;
	}
	if(space==COLORSPACE_SRGB){
		result = this->mapper->toRGB(result);
	}
	return result;
}

colorRaw LedLight::maskColor(colorRaw color){
	if(!(this->type & LIGHT_RED)) color.rgb.r = 0;
	if(!(this->type & LIGHT_GREEN)) color.rgb.g = 0;
	if(!(this->type & LIGHT_BLUE)) color.rgb.b = 0;
	if(!(this->type & LIGHT_WHITE)) color.white.w = 0;
	if(!(this->type & LIGHT_WHITEWARM)) color.cct.warm = 0;
	return color;
}

void LedLight::applySpecialColor(){
	if(this->special & (P_SPECIAL_RANDOM)){
		colorRaw color = HSV2RGB(randomColor());
		if(this->special & P_SPECIAL_DONTRESET){
			this->setColor(color,LIGHT_COLOR_DISPLAY, COLORSPACE_SRGB);
		}else{
			this->setColor(color,LIGHT_COLOR_USER, COLORSPACE_SRGB);
		}
	}
	if(this->special & P_SPECIAL_POWERDOWN && !(this->special & P_SPECIAL_ANIMATED)){
		this->setColor(colorBlack,LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
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
		this->setColor(this->userColor, LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
	}
}

void LedLight::setPowerState(powerState pwr){
	if(pwr == PS_OFF){
		this->special |= P_SPECIAL_POWERDOWN;
		animation *a = animCreate(myId, getColor(LIGHT_COLOR_DISPLAY, COLORSPACE_RAW),
			colorBlack,	16 << 8, (animType)ANIM_TEMPFX);
		animStart(a);
		
	}else if(pwr == PS_ON){
		this->special &= ~P_SPECIAL_POWERDOWN;
		
		animCreate(myId, getColor(LIGHT_COLOR_DISPLAY, COLORSPACE_RAW), 
			getColor(LIGHT_COLOR_USER, COLORSPACE_RAW), 16 << 8, (animType)ANIM_INTROCHAIN);
		
	}
}

