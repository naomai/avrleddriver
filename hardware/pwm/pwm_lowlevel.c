/*
 * pwm_lowlevel.c
 *
 * Created: 14.12.2019 14:54:42
 *  Author: bun
 */ 

#include <avr/interrupt.h>
#include "pwm_lowlevel.h"
#include "../../extender.h"
#include <string.h>

PWMOFFSET_VARTYPE shortPulsesTimingRoutine();
void maybeSwapBuffers();
void bufferSwapTableEntry(uint8_t index);
void bufferSwapAttribs();
void bufferSwapFinish();
void bufferSwapShortEvents();
void pwmTableNextItem();

// sorted tables, processed to form: delayFromLastEvent + allBitsValue
PWMBuffer outputBuffer; // currently "displayed" by PWM
PWMBuffer workBuffer; // currently processed in background
//#define outputBuffer workBuffer
uint8_t PWMTableOffset; // offset in outputBuffer for low-level PWM


volatile uint8_t swapPending = 0; // copy workBuffer to outputBuffer after current PWM cycle
uint8_t swapInProgress = 0; // set at the end of cycle when swapPending=1
uint8_t swapIndex = 0;

PWMPrecalcEvent *currentPrecalcEvent = &outputBuffer.precalcTable[0];
PWMPrecalcEvent *lastPrecalcEvent;
PWMOFFSET_VARTYPE PWMOffset = 0;
uint8_t PWMCycles = 0;


void initPwmLow(){
	//PWM Timer
	OCR1A = 10;
	TCCR1A &= ~(1<<COM1A0 | 1<< COM1A1);
	TCCR1B &= 0x07;
	TCCR1B |= 0x02 | (1<<WGM12);
}

ISR(TIMER1_COMPA_vect)
{
	PWMOFFSET_VARTYPE ticks;
	EXTENDER_VARTYPE extenderBits;
	OCR1AL=100;
	TIFR1&=(1<<OCF1A);
	if(outputBuffer.precalcCount==0){
		setExtenderValue(outputBuffer.extenderMask);
		OCR1A = PWM_RESOLUTION*PWM_SLOWDOWN;
		maybeSwapBuffers();
		return;
	}
	
	if(swapInProgress)
		bufferSwapTableEntry(swapIndex++);

	ticks = currentPrecalcEvent->ticks;
	extenderBits = currentPrecalcEvent->portMask;
	setExtenderValue(extenderBits);
	OCR1A = ticks*PWM_SLOWDOWN;
	TCNT1=0;

	
	pwmTableNextItem();
	/*swapPending = 0;
	ticks = PWM_RESOLUTION;*/
	
	

}

void pwmTableNextItem(){
	currentPrecalcEvent++;
	if(currentPrecalcEvent >= lastPrecalcEvent){
		currentPrecalcEvent = &outputBuffer.precalcTable[0];
		bufferSwapFinish();
		maybeSwapBuffers();
		PWMCycles++;
	}
}



void maybeSwapBuffers(){
	if(swapPending){
		//outputBuffer = workBuffer; // this takes ~900 CPU cycles, causing flicker
		//swapPending = 0;
		if(swapInProgress){
			//bufferSwapRest();
		}else{
			swapInProgress = 1;
			bufferSwapAttribs();
		}
	}
}

void bufferSwapTableEntry(uint8_t index){
	//outputBuffer.precalcTable[index] = workBuffer.precalcTable[index];
	outputBuffer.precalcTable[index].portMask = workBuffer.precalcTable[index].portMask;
	outputBuffer.precalcTable[index].ticks = workBuffer.precalcTable[index].ticks;
	swapInProgress = 2;
}

void bufferSwapAttribs(){
	outputBuffer.extenderMask = workBuffer.extenderMask;
	//outputBuffer.endingMask = workBuffer.endingMask;
	outputBuffer.precalcCount = workBuffer.precalcCount;
	//outputBuffer.bigIntervalsEndOffset = workBuffer.bigIntervalsEndOffset;
	//outputBuffer.smallIntervalsPointer = workBuffer.smallIntervalsPointer;
	outputBuffer.longestEventTicks = workBuffer.longestEventTicks;
	lastPrecalcEvent = &outputBuffer.precalcTable[outputBuffer.precalcCount];

}

void bufferSwapFinish(){
	if(swapInProgress==2){
		swapPending = 0;
		swapInProgress = 0;
		swapIndex = 0;
	}
}