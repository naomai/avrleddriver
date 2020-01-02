/*
 * radio.h
 *
 * Created: 09.12.2019 19:15:13
 *  Author: bun
 */ 


#ifndef RADIO_H_
#define RADIO_H_

typedef enum{
	rf_Nop = 0,			// i/o nothing to say
	rf_Register = 1,	// o   (u32 address) register myself to master /out of turn/
	rf_Unregister = 2,	// o    unregister 
	rf_Check = 3,		// i    are you there & do you have anything (else) to say
	rf_Request = 4,		// i/o (rfParam which) request data 
	//rf_Response = 5,	// i/o (byteStream) respond to request
	rf_ChangeState = 6,	// i/o (rfParam which, byteStream) request the other side to change its params
	rf_Event = 7		// o   (rfEvent type, ...) notify master about an event /out of turn/
} rfCommand;

typedef enum{
	rfParam_Config = 1, // number and types of strips
	rfParam_Settings = 2, // settings stream
	rfParam_State = 3 // current colors and state of this driver
}rfParam;

typedef enum {
	RFMODE_SLEEP,
	RFMODE_ORPHAN,
	RFMODE_REGISTERED,
	RFMODE_MASTER_WAITING,
	RFMODE_READY_TO_RESPOND
} rfMode_t;

void initRadio();
void radioSignIn(uint8_t address);
void radioTick();
void radioReportState(uint8_t reg);

#endif /* RADIO_H_ */