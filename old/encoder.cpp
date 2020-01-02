/*
 * encoder.c
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */


#include "../mcu.h"
#include "encoder.h"
#include <stdbool.h>
#include <util/delay.h>

#pragma GCC optimize("Os")

char lastEncoderState0,lastEncoderState1;
char lastButState=0;
bool state0changed=false;

void initEncoder(){
	ENCODER_DDR &= ~(ENCODER_PIN0_MASK|ENCODER_PIN1_MASK|ENCODER_BUT_MASK);
	ENCODER_PORT |= ENCODER_PIN0_MASK|ENCODER_PIN1_MASK|ENCODER_BUT_MASK;
	lastEncoderState0 = 0;
	#if !defined(DEBUG_SIMULATOR)
	_delay_ms(25); // stabilize encoder LPFs
	#endif
	checkEncoder();
}

encoderEvent checkEncoder(){
	char state0, state1;
	
	#ifdef DEBUG_SIMULATOR
	return ENC_NONE;
	#endif
	
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


bool checkEncoderButton(){
	#ifdef DEBUG_SIMULATOR
		return false;
	#endif
	return (~ENCODER_PIN) & ENCODER_BUT_MASK;
}