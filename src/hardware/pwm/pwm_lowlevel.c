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
#include <stdlib.h>

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
extern uint8_t PWMChannels;

//volatile uint8_t swapPending = 0; // copy workBuffer to outputBuffer after current PWM cycle
//uint8_t swapInProgress = 0; // set at the end of cycle when swapPending=1

volatile pwmBufferSwapStatus bufferSwapStatus;

uint8_t swapIndex = 0;

PWMPrecalcEvent *currentPrecalcEvent;
PWMPrecalcEvent *lastPrecalcEvent;
PWMOFFSET_VARTYPE PWMOffset = 0;
uint8_t PWMCycles = 0;


void initPwmLow(){
	//PWM Timer
	OCR1A = 10;
	OCR1B = PWM_RESOLUTION;
	TCCR1A &= ~(1<<COM1A0 | 1<< COM1A1);
	TCCR1B &= 0x07;
	TCCR1B |= 0x02 | (1<<WGM12);
	outputBuffer.precalcTable = calloc(PWMChannels, sizeof(PWMPrecalcEvent));
	workBuffer.precalcTable = calloc(PWMChannels, sizeof(PWMPrecalcEvent));
	currentPrecalcEvent = &outputBuffer.precalcTable[0];
}

ISR(TIMER1_COMPA_vect)
{
	PWMOFFSET_VARTYPE ticks;
	//uint8_t extenderBits[EXTENDER_BYTES];
	OCR1AL=100;
	TIFR1&=(1<<OCF1A);
	if(outputBuffer.precalcCount==0){
		setExtenderValue(outputBuffer.extenderMask);
		OCR1A = PWM_RESOLUTION*PWM_SLOWDOWN;
		maybeSwapBuffers();
		return;
	}
	
	if(bufferSwapStatus >= PBSWAP_INITIATED)
		bufferSwapTableEntry(swapIndex++);

	ticks = currentPrecalcEvent->ticks;
	//extenderBits = currentPrecalcEvent->portMask;
	setExtenderValue(currentPrecalcEvent->portMask);
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
	if(bufferSwapStatus == PBSWAP_REQUESTED){
		bufferSwapStatus = PBSWAP_INITIATED;
		bufferSwapAttribs();
		currentPrecalcEvent = &outputBuffer.precalcTable[0];
	}
}

void bufferSwapTableEntry(uint8_t index){
	//outputBuffer.precalcTable[index] = workBuffer.precalcTable[index];
	//outputBuffer.precalcTable[index].portMask = workBuffer.precalcTable[index].portMask;
	memcpy(outputBuffer.precalcTable[index].portMask,workBuffer.precalcTable[index].portMask,EXTENDER_BYTES);
	outputBuffer.precalcTable[index].ticks = workBuffer.precalcTable[index].ticks;
	bufferSwapStatus = PBSWAP_INPROGRESS;
}

void bufferSwapAttribs(){
	//outputBuffer.extenderMask = workBuffer.extenderMask;
	//outputBuffer.endingMask = workBuffer.endingMask;
	memcpy(outputBuffer.extenderMask,workBuffer.extenderMask,EXTENDER_BYTES);
	outputBuffer.precalcCount = workBuffer.precalcCount;
	//outputBuffer.bigIntervalsEndOffset = workBuffer.bigIntervalsEndOffset;
	//outputBuffer.smallIntervalsPointer = workBuffer.smallIntervalsPointer;
	//outputBuffer.longestEventTicks = workBuffer.longestEventTicks;
	lastPrecalcEvent = &outputBuffer.precalcTable[outputBuffer.precalcCount];

}

void bufferSwapFinish(){
	if(bufferSwapStatus==PBSWAP_INPROGRESS){
		bufferSwapStatus = PBSWAP_IDLE;
		swapIndex = 0;
	}
}