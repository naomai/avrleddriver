/*
 * pwm_lowlevel.h
 *
 * Created: 14.12.2019 14:57:27
 *  Author: bun
 */ 


#ifndef PWM_LOWLEVEL_H_
#define PWM_LOWLEVEL_H_

#include "../../config.h"
#include "../../types.h"
#include "../../display/LightConfig.h"

typedef struct {
	PWMOFFSET_VARTYPE offset;
	uint8_t bit;
} PWMEvent;

typedef struct {
	PWMOFFSET_VARTYPE delay; // ticks after previous event
	PWMOFFSET_VARTYPE ticks; // how many ticks to spend in this state
	uint8_t portMask[EXTENDER_BYTES];
} PWMPrecalcEvent;


typedef struct{
	PWMPrecalcEvent precalcTable[STRIPS_COUNT*MAX_CHANNELS_PER_LIGHT];
	
	uint8_t precalcCount;
	uint8_t extenderMask[EXTENDER_BYTES];
	//PWMOFFSET_VARTYPE bigIntervalsEndOffset;
	//uint8_t smallIntervalsPointer;
	//EXTENDER_VARTYPE endingMask;
	
	// (SPI_MULTIPLE_DEVICES) release SPI semaphore at this offset to prevent saturation
	//PWMOFFSET_VARTYPE longestEventTicks; 
} PWMBuffer;

typedef enum {
	PBSWAP_IDLE = 0,
	PBSWAP_REQUESTED = 1,
	PBSWAP_INITIATED = 2,
	PBSWAP_INPROGRESS = 3
}pwmBufferSwapStatus;

#ifdef __cplusplus
extern "C" {
#endif

void initPwmLow();

extern PWMBuffer workBuffer;
//extern volatile uint8_t swapPending;
extern volatile pwmBufferSwapStatus bufferSwapStatus;

#ifdef __cplusplus
}
#endif


#endif /* PWM_LOWLEVEL_H_ */