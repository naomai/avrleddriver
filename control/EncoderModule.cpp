/*
 * EncoderModule.cpp
 *
 * Created: 27.12.2019 14:49:13
 *  Author: bun
 */ 

#include "../config.h"

#include <util/delay.h>

#include "EncoderModule.h"


Encoder::Encoder() : Module(){
	this->tickPrescaler = 0;
		
	ENCODER_DDR &= ~(ENCODER_PIN0_MASK|ENCODER_PIN1_MASK|ENCODER_BUT_MASK);
	ENCODER_PORT |= ENCODER_PIN0_MASK|ENCODER_PIN1_MASK|ENCODER_BUT_MASK;
	lastEncoderState0 = 0;
	lastButState=0;
	state0changed=false;
	#if !defined(DEBUG_SIMULATOR)
	_delay_ms(25); // stabilize encoder LPFs
	#endif
	checkEncoder();
}
	
void Encoder::tick(){
	encoderEvent encoder;
	encoder = checkEncoder();
	switch(encoder){
		case ENC_CW:
			this->raiseEvent(EVENT_INPUT_DELTA, 1, 0);
			break;
		case ENC_CCW:
			this->raiseEvent(EVENT_INPUT_DELTA, -1, 0);
			break;
		case ENC_BUTDOWN:
			this->raiseEvent(EVENT_INPUT_KEY, KEYSTATE_DOWN, KEY_ENTER);
			break;
		case ENC_BUTUP:
			this->raiseEvent(EVENT_INPUT_KEY, KEYSTATE_UP, KEY_ENTER);
			break;
		default: 
			break;
	}
}
	
encoderEvent Encoder::checkEncoder(){
	char state0, state1;
		
	/*#ifdef DEBUG_SIMULATOR
	return ENC_NONE;
	#endif*/
		
	state0 = ENCODER_PIN & ENCODER_PIN0_MASK;
	if(state0 != lastEncoderState0){
		lastEncoderState0 = state0;
		state0changed=true;
	}
	state1 = ENCODER_PIN & ENCODER_PIN1_MASK;
	if(state1 != lastEncoderState1){
		lastEncoderState1 = state1;
		if(state0changed){
			if(!state1 == !state0)
			return ENC_CW;
			else
			return ENC_CCW;
				
		}
	}
	if((~ENCODER_PIN) & ENCODER_BUT_MASK){
		if(lastButState < 100){
			lastButState = 255;
			return ENC_BUTDOWN;
		}else{
			lastButState = 255;
		}
		}else{
		if(lastButState == 100){
			lastButState = 0;
			return ENC_BUTUP;
		}else if(lastButState > 0){
			lastButState -= 1;
		}
	}

	return ENC_NONE;
}


bool Encoder::checkEncoderButton(){
	#ifdef DEBUG_SIMULATOR
	return false;
	#endif
	return (~ENCODER_PIN) & ENCODER_BUT_MASK;
}
