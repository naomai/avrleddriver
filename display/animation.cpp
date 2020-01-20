/*
 * animation.c
 *
 * Created: 13.11.2019 12:42:18
 *  Author: bun
 */ 

#include "animation.h"
#include "../LedDriver.h"
#include "color.h"
#include "../types.h"
#include "../debug.h"
#include "../modular/Dispatcher.h"

#include <avr/pgmspace.h>

#pragma GCC optimize("Os")

void customAnimTriggers(animation *a);
void animAdvanceFrame(animation *a);
void animFinished(animation *a);
void animCheckIntroChain();

const uint8_t PROGMEM scurveLut[256]={
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x04,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x07,
	0x07,0x08,0x08,0x09,0x09,0x09,0x0A,0x0A,0x0B,0x0B,0x0C,0x0C,0x0D,0x0D,0x0E,0x0E,
	0x0F,0x0F,0x10,0x11,0x12,0x12,0x13,0x14,0x15,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,
	0x1C,0x1D,0x1F,0x20,0x21,0x22,0x24,0x25,0x26,0x28,0x29,0x2B,0x2C,0x2E,0x30,0x31,
	0x33,0x35,0x37,0x39,0x3B,0x3D,0x3F,0x41,0x43,0x45,0x47,0x49,0x4C,0x4E,0x50,0x53,
	0x55,0x58,0x5A,0x5D,0x5F,0x62,0x64,0x67,0x6A,0x6C,0x6F,0x72,0x75,0x77,0x7A,0x7D,
	0x80,0x82,0x85,0x88,0x8A,0x8D,0x90,0x93,0x95,0x98,0x9B,0x9D,0xA0,0xA2,0xA5,0xA7,
	0xAA,0xAC,0xAF,0xB1,0xB3,0xB6,0xB8,0xBA,0xBC,0xBE,0xC0,0xC2,0xC4,0xC6,0xC8,0xCA,
	0xCC,0xCE,0xCF,0xD1,0xD3,0xD4,0xD6,0xD7,0xD9,0xDA,0xDB,0xDD,0xDE,0xDF,0xE0,0xE2,
	0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEA,0xEB,0xEC,0xED,0xED,0xEE,0xEF,0xF0,
	0xF0,0xF1,0xF1,0xF2,0xF2,0xF3,0xF3,0xF4,0xF4,0xF5,0xF5,0xF6,0xF6,0xF6,0xF7,0xF7,
	0xF8,0xF8,0xF8,0xF8,0xF9,0xF9,0xF9,0xFA,0xFA,0xFA,0xFA,0xFA,0xFB,0xFB,0xFB,0xFB,
	0xFB,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD,
	0xFD,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF
};

animation currentAnims[10];

extern Dispatcher *dispatcher;
extern LedDriver *lights;

animation* animCreate(uint8_t stripId, colorRaw from, colorRaw to, uint16_t speed, animType tag){
	animation *newAnim;
	LedLight *strip = lights->getLightById(stripId);
	//_logf("Create for strip %i", stripId);
	for(uint8_t i=0; i<count(currentAnims); i++){
		if(currentAnims[i].tag == ANIM_NONE || !(~currentAnims[i].progress)){
			newAnim = &currentAnims[i];
			newAnim->stripId = stripId;
			newAnim->from = from;
			newAnim->to = to;
			newAnim->speed = speed;
			newAnim->tag = tag;
			newAnim->progress = 0;
			strip->setColor(from, LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
			if(newAnim->tag==ANIM_INTROCHAIN){
				strip->special |= P_SPECIAL_ANIMATED;
			}
			
			return newAnim;
		}
	}
	//no free anim spots
	_log("Unable to create anim - reached limit");
	strip->setColor(to, LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
	return 0;
}

void animStart(animation *a){
	if(!a->progress){
		for(uint8_t i=0; i<count(currentAnims); i++){ // cancel other anims running on this light
			animation *animToCancel = &currentAnims[i];
			if(animToCancel != a && animToCancel->stripId == a->stripId && 
			  (animIsActive(animToCancel) || animToCancel->tag==ANIM_INTROCHAIN)
			){
					animCancel(&currentAnims[i]);
			}
		}
		lights->getLightById(a->stripId)->special |= P_SPECIAL_ANIMATED;
		a->progress=1;
		//_logf("Started strip %i", a->stripId);
		eventDescriptor ev;
		ev.type = EVENT_ANIM_START;
		ev.lbyte = a->tag;
		ev.hbyte = a->stripId;
		dispatcher->queue->pushEvent(ev);
	}
}

void animCancel(animation *a){
	a->progress = ~0;
	a->tag = ANIM_NONE;
	lights->getLightById(a->stripId)->special &= ~(P_SPECIAL_ANIMATED);
}

void animAdvanceAll(){
	animation *anim;
	for(uint8_t i=0; i<count(currentAnims); i++){
		anim = &currentAnims[i];
		animAdvanceFrame(anim);
	}
	animCheckIntroChain();
}

void animAdvanceFrame(animation *a){
	colorRaw newColor;
	uint16_t newProgress;
	LedLight *strip;
	uint8_t blendValue;
		
	if(!animIsActive(a))
		return;
	
	#ifdef DEBUG_NOANIM
		if(a->speed){
			a->progress = 1;
			newProgress = 0;
		}
	#else
		newProgress = a->progress + a->speed;
	#endif
	
	strip = lights->getLightById(a->stripId);
	
	if(a->progress > newProgress){ // progress counter overflow
		if(animIsLooping(a)){ // another round
			a->progress = 1;			
		}else{ // animation is finished
			strip->setColor(a->to, LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
			if(a->tag != ANIM_TEMPFX && a->tag != ANIM_INTROCHAIN){
				strip->setColor(a->to, LIGHT_COLOR_USER, COLORSPACE_RAW);
			}
			strip->special &= ~(P_SPECIAL_ANIMATED);
			a->progress = ~0;
			a->tag = ANIM_NONE;
			animFinished(a);
		}
	}
	
	if(animIsActive(a)){
		a->progress = newProgress;
		
		if(a->tag == ANIM_LOOP_ALTERNATING){
			 if(newProgress > 32768){
				 newProgress = 32768-newProgress;
			 }
			 newProgress <<= 1;
		}
		
		blendValue = pgm_read_byte(&scurveLut[newProgress >> 8]);
		newColor = blendColor(a->from, a->to, blendValue);

		strip->setColor(newColor, LIGHT_COLOR_DISPLAY, COLORSPACE_RAW);
	}
	customAnimTriggers(a);
}

void animFinished(animation *a){
	//_logf("Finished strip %i", a->stripId);
	eventDescriptor ev;
	ev.type = EVENT_ANIM_FINISH;
	ev.lbyte = a->tag;
	ev.hbyte = a->stripId;
	dispatcher->queue->pushEvent(ev);
}

bool animIsActive(animation *a){
	return a && a->tag != ANIM_NONE && a->progress && (~a->progress);
}

bool animIsLooping(animation *a){
	return a->tag & ANIM_LOOP_MASK;	
}
void customAnimTriggers(animation *a){
	if(a->tag == ANIM_INTROCHAIN && (a->progress>>8) > 128){
		for(uint8_t i=a->stripId+1; i<count(currentAnims); i++){
			if(currentAnims[i].tag == ANIM_INTROCHAIN && !animIsActive(&currentAnims[i])){
				animStart(&currentAnims[i]);
				break;
			}
		}
	}
}

void animCheckIntroChain(){
	for(uint8_t i=0; i<count(currentAnims); i++){
		if(currentAnims[i].tag == ANIM_INTROCHAIN){
			if(!animIsActive(&currentAnims[i])) {
				animStart(&currentAnims[i]);
			}
			break;
		}
	}
	
}