/*
 * multispi.c
 *
 * Created: 18.12.2019 12:00:17
 *  Author: bun
 */ 

// this file is unused since a thought came into my mind:
// "what if UART could be used as SPI?"

#include "config.h"
#include "spishare.h"
#include <stdio.h>
#include <avr/interrupt.h>
#include "extender.h"

volatile uint8_t interruptsDisabledCarry = 0;
volatile uint8_t spiSemaphore = 0;
volatile uint8_t spiCurrentThread = 0;
uint8_t SPIConfigs[SPI_DEVICES_COUNT];

void spiConfigAdd(uint8_t threadId, uint8_t regSPCR){
	/*#ifdef SPI_MULTIPLE_DEVICES
	SPIConfigs[threadId] = regSPCR;
	#else*/
	SPCR =  regSPCR;
	//#endif
}

/*void spiConfigSwitch(uint8_t threadId){
	#ifdef SPI_MULTIPLE_DEVICES
	if(threadId != spiCurrentThread){
		SPCR = SPIConfigs[threadId];
		spiCurrentThread=threadId;
	}
	#endif
}*/

// "big thread" is the routine for most important SPI device (e.g. PWM)
// "small threads" are non-critical routines allowed in free time-slot scheduled by "big thread" (e.g. RF transmitter)
// big uses spiBig..(), smalls use spiWait() followed by spiSmall..()
void spiBigThreadLock(){
	//#ifdef SPI_MULTIPLE_DEVICES
	spiSemaphore = 1;
	//#endif
}

void spiBigThreadUnlock(){
	//#ifdef SPI_MULTIPLE_DEVICES
	spiSemaphore = 0;
	//#endif
}

void spiWait(){
	//#ifdef SPI_MULTIPLE_DEVICES
	while(spiSemaphore);
	//#endif
}
bool spiCheckLock(){
	return spiSemaphore;
}

void spiSmallThreadLock(){
	/*#ifdef SPI_MULTIPLE_DEVICES
	// store current state of glob. int. flag before clearing
	asm(
	"brid intdis\n"
	"cli\n"
	"ldi %[carry], 1\n"
	"intdis:" :
	[carry] "=r" (interruptsDisabledCarry) :
	);
	#ifdef EXTENDER_USE_SN74HC164 
	SN74HC164_CLR_PORT &= ~(1<<SN74HC164_CLR_BIT);
	#endif
	#endif*/
}

void spiSmallThreadUnlock(){
	/*#ifdef SPI_MULTIPLE_DEVICES
	#ifdef EXTENDER_USE_SN74HC164
	SN74HC164_CLR_PORT |= 1<<SN74HC164_CLR_BIT;
	#endif
	if(interruptsDisabledCarry){
		interruptsDisabledCarry=0;
		sei();
	}
	#endif*/
}