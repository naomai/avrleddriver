/*
 * ledstrip.c
 *
 * Created: 13.11.2019 12:49:33
 *  Author: bun
 */ 

#include "../config.h"
#include "ledstrips.h"
#include "../hardware/pwm/pwm.h"
#include "../hardware/LedHardware.h"

//extern const lightConfig PROGMEM lightsConfig[STRIPS_COUNT] = STRIPS_CONFIG;

LedLight *strips[STRIPS_COUNT];
LedHardware *ledHal;


void initLights(){
	lightConfig cfg;
	light_s *stateHW;
	ledHal = new LedHardware();
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		memcpy_P(&cfg, &lightsConfig[i], sizeof(lightConfig));
		stateHW = ledHal->registerLight(&cfg);
		strips[i] = new LedLight(stateHW);
		//strips[i]->setColor(colorWhite,LIGHT_COLOR_USER);
	}
	
	
	
	
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		applySpecialColor(strips[i]);
		resetTempColor(strips[i]);
		calculateRGB(strips[i]);
	}
	
}

LedLight* stripById(uint8_t stripId){
	return strips[stripId];
}

void lightsUpdate(){
	ledHal->tick();
}




void setStripRGB(LedLight *s, colorRaw rgb){
	/*s->rawColor = rgb;
	applySpecialColor(s);
	s->userColor = RGB2HSV(rgb);*/
	s->setColor(rgb, LIGHT_COLOR_DISPLAY);
	s->applySpecialColor();
	s->setColor(rgb, LIGHT_COLOR_USER);

}

void setStripSpecial(LedLight *s, uint8_t special){
	s->setSpecialAttribute(special);
}

void applySpecialColor(LedLight *s){
	s->applySpecialColor();
}

void resetTempColor(LedLight *s){
	s->resetTempColor();
}

void calculateRGB(LedLight *s){
	#ifdef DEBUG_RAINBOWTEST
	s->setColor(HSV2RGB(randomColor()),LIGHT_COLOR_DISPLAY);
	#endif
	/*uint8_t special = s->special;
	
	
	if(!(special & (P_SPECIAL_ANIMATED>>24))){
		s->rawColor = HSV2RGB(s->tempColor);
		//s->special = special;
	}*/
	
}