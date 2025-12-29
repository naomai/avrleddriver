/*
 * pwm.c
 *
 * Created: 14.12.2019 14:44:31
 *  Author: bun
 */ 

#include "pwm.h"
#include "pwm_lowlevel.h"
#include "../LedHardware.h"
#include <string.h>
#include <stdlib.h>

#pragma GCC optimize("Os")

const uint16_t PROGMEM gammaLut[256]={
	0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x03,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x0A,0x0A,
	0x0B,0x0C,0x0D,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
	0x1A,0x1C,0x1D,0x1E,0x1F,0x21,0x22,0x23,0x25,0x26,0x28,0x29,0x2B,0x2C,0x2E,0x30,
	0x31,0x33,0x35,0x36,0x38,0x3A,0x3C,0x3E,0x40,0x42,0x44,0x46,0x48,0x4A,0x4C,0x4E,
	0x50,0x52,0x55,0x57,0x59,0x5C,0x5E,0x60,0x63,0x65,0x68,0x6A,0x6D,0x70,0x72,0x75,
	0x78,0x7A,0x7D,0x80,0x83,0x86,0x89,0x8C,0x8F,0x92,0x95,0x98,0x9B,0x9E,0xA1,0xA4,
	0xA8,0xAB,0xAE,0xB2,0xB5,0xB9,0xBC,0xC0,0xC3,0xC7,0xCA,0xCE,0xD2,0xD5,0xD9,0xDD,
	0xE1,0xE5,0xE9,0xED,0xF1,0xF5,0xF9,0xFD,0x101,0x105,0x109,0x10D,0x112,0x116,0x11A,0x11F,
	0x123,0x128,0x12C,0x131,0x135,0x13A,0x13E,0x143,0x148,0x14D,0x151,0x156,0x15B,0x160,0x165,0x16A,
	0x16F,0x174,0x179,0x17E,0x183,0x189,0x18E,0x193,0x199,0x19E,0x1A3,0x1A9,0x1AE,0x1B4,0x1B9,0x1BF,
	0x1C5,0x1CA,0x1D0,0x1D6,0x1DB,0x1E1,0x1E7,0x1ED,0x1F3,0x1F9,0x1FF,0x205,0x20B,0x211,0x217,0x21E,
	0x224,0x22A,0x231,0x237,0x23D,0x244,0x24A,0x251,0x257,0x25E,0x265,0x26B,0x272,0x279,0x280,0x286,
	0x28D,0x294,0x29B,0x2A2,0x2A9,0x2B0,0x2B8,0x2BF,0x2C6,0x2CD,0x2D4,0x2DC,0x2E3,0x2EB,0x2F2,0x2F9,
	0x301,0x309,0x310,0x318,0x320,0x327,0x32F,0x337,0x33F,0x347,0x34F,0x356,0x35E,0x367,0x36F,0x377,
	0x37F,0x387,0x38F,0x398,0x3A0,0x3A8,0x3B1,0x3B9,0x3C2,0x3CA,0x3D3,0x3DC,0x3E4,0x3ED,0x3F6,0x3FF,
};


// table of LED turnoff events: time + bitNumber
static PWMEvent *PWMTable;
// table pointing to elements from PWMTable in sorted order
static PWMEvent** PWMTableSorted;

static uint8_t PWMTableCount;
uint8_t PWMChannels = 0;

//static void pwmMoveShortEvents();
static void addPWMItem(uint8_t offset, uint8_t bit);

#define MASK_SET_BIT(m,b) m[(b)>>3] |= (1 << ((b)&0x07))
#define MASK_CLEAR_BIT(m,b) m[(b)>>3] &= ~(1 << ((b)&0x07))
#define MASK_BIT(m,b) (m[(b)>>3] & (1 << ((b)&0x07)))

void initPwm(){
	PWMTable = calloc(PWMChannels, sizeof(PWMEvent));
	PWMTableSorted = calloc(PWMChannels, sizeof(PWMEvent*));
	initPwmLow();	
}

bool pwmIsReady(){
	return !bufferSwapStatus;
	//return true;
}


void pwmPrepareStart() {
	PWMTableCount = 0;
	//PWMTableOffset = 0;
	//workBuffer.extenderMask = 0;
	memset(workBuffer.extenderMask, 0, EXTENDER_BYTES);
}


void pwmBufferLight(light_s *s){
	entityType type = s->hardwareConfig.type;
	
	if(type&LIGHT_RED)
		addPWMItem(s->color.rgb.r, s->hardwareConfig.pins[0]);
	if(type&LIGHT_GREEN)
		addPWMItem(s->color.rgb.g, s->hardwareConfig.pins[1]);
	if(type&LIGHT_BLUE)
		addPWMItem(s->color.rgb.b, s->hardwareConfig.pins[2]);
	if(type&LIGHT_WHITE)
		addPWMItem(s->color.white.w, s->hardwareConfig.pins[3]);
	if(type&LIGHT_WHITEWARM)
		addPWMItem(s->color.cct.warm, s->hardwareConfig.pins[4]);
}

void pwmPrepareFinish() {
	PWMEvent *currentEvent = PWMTableSorted[0], *nextEvent;
	PWMPrecalcEvent *precalcPrev, *precalcCurrent;
	uint8_t precalcCount = 1;
	
	precalcPrev = &workBuffer.precalcTable[0];
	precalcPrev->delay = 0;
	precalcPrev->ticks = 0;
	memcpy(&precalcPrev->portMask[0], &workBuffer.extenderMask[0], EXTENDER_BYTES);
	precalcCurrent = &workBuffer.precalcTable[1];
	precalcCurrent->delay = 0;
	
	uint8_t currentMask[EXTENDER_BYTES];
	memcpy(currentMask, &workBuffer.extenderMask[0], EXTENDER_BYTES);
	//uint16_t prevMask=currentMask;
	//uint16_t nextMask;
	
	PWMOFFSET_VARTYPE currentOffset=0, previousOffset = 0;
	
	for (uint8_t i = 0; i < PWMTableCount; i++) {
		//if(currentMask == 0) break;
		
		currentEvent = PWMTableSorted[i];

		MASK_CLEAR_BIT(currentMask, currentEvent->bit);
		
		// set only once for offset/mask
		if(!precalcCurrent->delay){
			currentOffset = currentEvent->offset;
			precalcCurrent->delay = currentOffset - previousOffset;
			precalcCurrent->ticks = 0;

		}
		//precalcCurrent->portMask = currentMask;		
		memcpy(precalcCurrent->portMask, currentMask, EXTENDER_BYTES);
		
		/*if(currentMask == prevMask) {
			//precalcPrev->ticks += precalcCurrent->delay;
			//previousOffset = currentOffset;
			continue;
		}*/

		
		//foreshadow - merge events happening at the same time or having the same bit mask
		nextEvent = PWMTableSorted[i+1];
		
		bool sameMask = !MASK_BIT(currentMask, nextEvent->bit);
		MASK_CLEAR_BIT(currentMask, nextEvent->bit);
		
		bool sameOffset = nextEvent->offset == currentOffset;
		//bool sameMask = nextMask == currentMask;
		bool lastIteration = (i >= PWMTableCount - 1);
		
		// move to next empty position in precalc table
		if((!sameOffset && !sameMask) || lastIteration){
			precalcPrev->ticks = precalcCurrent->delay;
			
			
			precalcPrev = precalcCurrent;
			precalcCurrent++;
			previousOffset = currentOffset;
			precalcCount++;
			if(!lastIteration){ // for "set only once"
				precalcCurrent->delay = 0;
			}
		}
		//prevMask = currentMask;
	}
	precalcPrev->ticks = PWM_RESOLUTION-currentOffset;
	
	
	//workBuffer.endingMask = currentMask;
	//pwmMoveShortEvents();
	workBuffer.precalcCount = precalcCount;
	
	bufferSwapStatus = PBSWAP_REQUESTED;
	
}
/*
void pwmMoveShortEvents(){
	// short pulses moved at bottom, processed with more precise routine
	uint8_t precalcCount = workBuffer.precalcCount;
	uint8_t bigCount = precalcCount, smallCount = 0;
	uint8_t bottomPtr = count(workBuffer.precalcTable);
	PWMOFFSET_VARTYPE bigIntervalsLength = 0;
	PWMPrecalcEvent *precalcCurrent;
		
	uint8_t currentIdx = 0;
	while(currentIdx < bigCount){
		precalcCurrent = &workBuffer.precalcTable[currentIdx];
		if(precalcCurrent->ticks < PWM_SHORTPULSE){
			workBuffer.precalcTable[--bottomPtr] = *precalcCurrent;
			//memcpy(&workBuffer.precalcTable[--bottomPtr], precalcCurrent, sizeof(PWMPrecalcEvent));
			bigCount--;
			*precalcCurrent = workBuffer.precalcTable[bigCount];
			//memcpy(precalcCurrent, &workBuffer.precalcTable[bigCount], sizeof(PWMPrecalcEvent));
			smallCount++;
		}else{
			bigIntervalsLength += precalcCurrent->ticks;
			currentIdx++;
		}
	}
	
	PWMOFFSET_VARTYPE longestTicks=PWM_LONGPULSE, longestOffset=0, offset=0;
	for(uint8_t i=0; i<bigCount; i++){
		precalcCurrent = &workBuffer.precalcTable[i];
		if(precalcCurrent->ticks > longestTicks){
			longestTicks = precalcCurrent->ticks;
			longestOffset = offset;
		}
		offset += precalcCurrent->ticks;
	}
		
	workBuffer.bigIntervalsEndOffset = bigIntervalsLength;
	workBuffer.smallIntervalsPointer = bottomPtr;
	workBuffer.longestEventOffset = longestOffset;
	workBuffer.longestEventTicks = longestTicks;
}
*/

void addPWMItem(uint8_t offset, uint8_t bit)   {
	PWMOFFSET_VARTYPE offsetCorrected;
	
	//gamma correction & normalization to PWM resolution
	offsetCorrected = pgm_read_word(&gammaLut[offset]);
	
	if (offsetCorrected == 0) return; // never turns on
	//workBuffer.extenderMask |= 1 << bit;
	MASK_SET_BIT(workBuffer.extenderMask, bit);
	
	if (offsetCorrected == PWM_RESOLUTION-1) return; // never turns off
	
	
	PWMEvent newItem;
	newItem.offset = offsetCorrected;
	newItem.bit = bit;
	
	uint8_t ptCount = PWMTableCount;
	
	PWMTable[ptCount] = newItem;
	PWMTableSorted[ptCount] = &PWMTable[ptCount];

	if(ptCount){
		//insertion sort
		for (int8_t i = ptCount-1; i >= 0; i--) {
			if ( newItem.offset < PWMTableSorted[i]->offset) {
				PWMTableSorted[i + 1] = PWMTableSorted[i];
				PWMTableSorted[i] = &PWMTable[ptCount];
			}
			else {
				break;
			}
		}
	}
	PWMTableCount++;
	
}