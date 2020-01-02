/*
 * random.c
 *
 * Created: 29.10.2019 17:41:59
 *  Author: bun
 */ 

#include "mcu.h"
#include "random.h"
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include "debug.h"
#include "config.h"

// uses xorshift by George Marsaglia
#pragma GCC optimize("Os")


struct xorshift32_state {
	uint32_t a;
};

uint32_t *ramEntropyPool; 
uint8_t ramEntropyOffset=0;

uint32_t xorshift32(struct xorshift32_state *state);
void rngPrepareAdc();
void rngSeedFromAdc(char shift, bool shiftWhileWaiting);
void rngRestoreAdc();
void entropyFromSRAM();

struct xorshift32_state myState;
uint8_t prevADMUX, prevADCSRA;

void initRandom() {
	if(!myState.a){
		myState.a = 0x63fefa2a;
	}
	ramEntropyPool = (uint32_t*)malloc(RANDOM_SRAM_ENTROPY_POOL_SIZE*sizeof(int32_t));
	//_logf("Entropy beforeRam: %lx", myState.a);
	//dumpRam(ramEntropyPool,	ramEntropyPool+RANDOM_SRAM_ENTROPY_POOL_SIZE);
	//dumpRam(1972, 2048);

	entropyFromSRAM();
	//_logf("Entropy afterRam: %lx", myState.a);
	rngPrepareAdc();
	rngSeedFromAdc(0,false);
	rngSeedFromAdc(16,false);
	rngSeedFromAdc(5,true);
	rngSeedFromAdc(28,false);
	//_logf("Entropy adc1: %lx", myState.a);
	for(uint8_t i=0; i < 30+(random8()%20); i++){
		rngSeedFromAdc(random8() % 27, false);
		//_delay_ms(5);
	}

	//rngRestoreAdc();
	//_logf("Entropy adc2: %lx", myState.a);
}

uint8_t random8(){
	return xorshift32(&myState) & 0xFF;
}
uint16_t random16(){
	return xorshift32(&myState) & 0xFFFF;
}
uint32_t random32(){
	return xorshift32(&myState);
}

void randomFeedEntropyDword(int32_t entropy){
	myState.a ^= entropy;
}

uint32_t randomFeedEntropy(){
	//rngPrepareAdc();
	rngSeedFromAdc(0, true);
	//rngRestoreAdc();
	//influence entropyFromSRAM() during quick reboots
	ramEntropyPool[ramEntropyOffset]=myState.a;
	ramEntropyOffset++;
	if(ramEntropyOffset >= RANDOM_SRAM_ENTROPY_POOL_SIZE)
		ramEntropyOffset=0;

	return myState.a;
}

void entropyFromSRAM(){
	for (uint32_t *i = (uint32_t*)0x000; i < (uint32_t*)0x300; i++){
		 myState.a ^= (*i);
	}
	for(uint8_t i = 0; i<RANDOM_SRAM_ENTROPY_POOL_SIZE; i++){
		myState.a ^= ramEntropyPool[i];	
	}
}

void rngPrepareAdc(){
	prevADMUX = ADMUX;
	prevADCSRA = ADCSRA;
	ADMUX = (1<<REFS0) /*| (1<<REFS1)*/; // adc0
	ADCSRA = (1<<ADEN);
	DDRC &= ~(1<<PC0);
	PORTC &= ~(1<<PC0);
	#ifdef DIDR0
	DIDR0 |= (1<<ADC0D);
	#endif
	//DDRD &= ~(1<<PD6)
}

void rngRestoreAdc(){
	ADMUX = prevADMUX;
	ADCSRA = prevADCSRA;
	#ifdef DIDR0
	DIDR0 &= ~(1<<ADC0D);
	#endif
}

void rngSeedFromAdc(char shift, bool shiftWhileWaiting){
	#if !defined(DEBUG_SIMULATOR)
	ADCSRA |= (1<<ADSC);
	if(shiftWhileWaiting){
		random32();
	}
	while(!bit_is_set(ADCSRA, ADIF));
	myState.a ^= ((uint32_t)ADC) << shift;
	//_logf("Entropy adc: %x\n", ADC);
	#endif
}


/* The state word must be initialized to non-zero */
uint32_t xorshift32(struct xorshift32_state *state)
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}
