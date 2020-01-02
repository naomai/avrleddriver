/*
 * Radio.h
 *
 * Created: 28.12.2019 13:00:13
 *  Author: bun
 */ 


#ifndef RADIO_H_
#define RADIO_H_


#include <stdbool.h>
#include "../modular/Module.h"

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

typedef enum{
	rfRegisterSet_Temporary = 0,
	rfRegisterSet_Permanent = 1
}rfRegisterSet;

typedef enum {
	RFMODE_SLEEP,
	RFMODE_ORPHAN,
	RFMODE_REGISTERED,
	RFMODE_MASTER_WAITING,
	RFMODE_READY_TO_RESPOND
} rfMode_t;

#define PACKET_LEN 32
#define RF_TIMEOUT 15 // sec

class Radio : public Module{
	public:
	Radio();
	~Radio();
	void signIn(uint8_t myId);
	void frameTick();
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	
	protected:
	rfMode_t rfMode;
	uint8_t myDeviceId;
	uint8_t responseBuffer[PACKET_LEN];
	uint8_t responseLength;
	uint16_t ticksSinceLastComm;
	uint8_t waitFrames;
	bool responsePrepared;
	
	void reportState(uint8_t reg);
	void processCommand(uint8_t * data);
	void processRequest(uint8_t * data);
	void processChangeState(uint8_t * data);
	void respondState(uint8_t reg);
	void sendResponse();
};


#endif /* RADIO_H_ */