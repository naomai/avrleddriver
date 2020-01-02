/*
 * radio.c
 *
 * Created: 09.12.2019 19:09:31
 *  Author: bun
 */ 

#include <stdbool.h>
#include "../config.h"
#include "radio_legacy.h"
#include "../display/LightConfig.h"
#include "../display/light/LedLight.h"
#include "../LedDriver.h"
#include "../debug.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../display/animation.h"
#include "string.h"
#include "../random.h"
//#include "multistream.h"


#define ADDR_PREFIX RFM73_ADDRESS_BASE
#define ADDR_MASTER (ADDR_PREFIX)
#define PACKET_LEN 32

#include "../hardware/RFM73.h"

void radioProcessCommand(uint8_t * data);
void radioProcessRequest(uint8_t * data);
void radioProcessChangeState(uint8_t * data);
void radioRespondState(uint8_t reg);
void radioSendResponse();


rfMode_t rfMode = RFMODE_SLEEP;

#define RF_TIMEOUT 15 // sec

uint8_t myDeviceId = 1;
uint8_t responseBuffer[PACKET_LEN];
uint8_t responseLength = 0;
uint8_t waitTicks = 50;
uint16_t ticksSinceLastComm = 0;
bool responsePrepared = false;

extern const lightConfig PROGMEM lightsConfig[STRIPS_COUNT];
extern LedDriver *lights;

void initRadio(){
	RFM73_Init();
	RFM73_SetChannel(RFM73_CHANNEL);
}

void radioTick(){
	uint8_t rfBuffer[PACKET_LEN];
	//bool status;
	ticksSinceLastComm++;
	
	if(--waitTicks){
		return;
	}
	waitTicks = 10;
	
	switch(rfMode){
		
		case RFMODE_ORPHAN:
		//greeting
			
			_log("Notice me sempai");
			responseBuffer[0] = rf_Register;
			responseBuffer[1] = myDeviceId;
			if(RFM73_Transmit(ADDR_MASTER, responseBuffer, 2)){
				//_log("Greeting sent");
				rfMode = RFMODE_REGISTERED;
				waitTicks = 2;
				ticksSinceLastComm = 0;
			}else{
				waitTicks = 2 + (random8() & 7); //avoid overlapping requests from two driver modules
			}
			break;
		case RFMODE_REGISTERED:
			if(RFM73_IsDataAvailable()){
				//status = RFM73_IsAckPayloadSent();				
				uint8_t bytes = RFM73_ReadData(rfBuffer);
				rfMode = RFMODE_MASTER_WAITING;
				//_logf("RX bs=%i cmd=%i", bytes, rfBuffer[0]);
				radioProcessCommand(rfBuffer);
				ticksSinceLastComm = 0;
			}
			if(ticksSinceLastComm > RF_TIMEOUT * FPS){
				rfMode = RFMODE_ORPHAN;
			}
			waitTicks = 3;
			break;
		case RFMODE_READY_TO_RESPOND:
			RFM73_Transmit(ADDR_MASTER, responseBuffer, responseLength);
			/*if(RFM73_Transmit(ADDR_MASTER, responseBuffer, responseLength)){*/
			rfMode = RFMODE_REGISTERED;
			responsePrepared = false;
			//}
			waitTicks = 3;
			break;
		case RFMODE_SLEEP:
			break;
		case RFMODE_MASTER_WAITING:
			// invalid state
			rfMode = RFMODE_REGISTERED;
			break;
		default:
			break;
	}
	
	
}

void radioSignIn(uint8_t myId){
	myDeviceId = myId;
	RFM73_ListenStart(ADDR_PREFIX | myDeviceId);
	rfMode = RFMODE_ORPHAN;
}

void radioProcessCommand(uint8_t * data){
	rfCommand cmd = (rfCommand)data[0];
	switch(cmd){
		case rf_Request:
			//_logf("Req %i", data[1]);
			radioProcessRequest(data);
			break;
		case rf_ChangeState:
			//_logf("Req %i", data[1]);
			radioProcessChangeState(data);
			break;
		case rf_Check:
			if(responsePrepared){
				rfMode = RFMODE_READY_TO_RESPOND;
				waitTicks = 2;
			}
	}
}

void radioProcessRequest(uint8_t * data){
	rfParam param = (rfParam)data[1];
	responseBuffer[0] = rf_ChangeState;
	responseBuffer[1] = param;
	switch(param){
		case rfParam_Config:
			//_log("PARAMREQ");
			responseBuffer[2] = STRIPS_COUNT;
			responseBuffer[3] = MAX_CHANNELS_PER_LIGHT;
			for(uint8_t i=0; i<STRIPS_COUNT; i++){
				responseBuffer[4+i] = pgm_read_byte(&lightsConfig[i].type);
				responseLength = 5+i;
			}
			radioSendResponse();
			break;
		case rfParam_State:
			radioRespondState(data[2]);
			break;
	}
	
}

void radioProcessChangeState(uint8_t * data){
	rfParam reg = (rfParam)data[1];
	if(reg < 128 && reg < STRIPS_COUNT){
		animation * anim;
		uint8_t speed;
		colorRaw newColor;
		LedLight* strip = lights->getLightById(reg);
		
		speed = data[2];
		memcpy(&newColor, &data[3], sizeof(colorRaw));
	
		anim = animCreate(reg, strip->getColor(LIGHT_COLOR_DISPLAY), newColor, speed << 8, ANIM_REMOTE);
		animStart(anim);
	}
}


void radioReportState(uint8_t reg){
	responseBuffer[0] = rf_ChangeState;
	responseBuffer[1] = rfParam_State;
	radioRespondState(reg);
}
	

void radioRespondState(uint8_t reg){
	//_logf("STATERQ %i", reg);
	responseBuffer[2] = reg;
	if(reg < 128 && reg < STRIPS_COUNT){
		LedLight *s = lights->getLightById(reg);
		uint8_t bufferOffset = 3;
		colorRaw col = s->getColor(LIGHT_COLOR_USER);
		memcpy(&responseBuffer[bufferOffset], &col, sizeof(col));
		bufferOffset += sizeof(col);
		col = s->getColor(LIGHT_COLOR_DISPLAY);
		memcpy(&responseBuffer[bufferOffset], &col, sizeof(col));
		bufferOffset += sizeof(col);
		responseBuffer[bufferOffset] = s->special;
		bufferOffset += 1;
		responseLength = bufferOffset;
		radioSendResponse();
	}	
}

void radioSendResponse(){
	if(rfMode == RFMODE_MASTER_WAITING){
		rfMode=RFMODE_READY_TO_RESPOND;
		waitTicks = 2;
	}
	responsePrepared = true;
	// ACK+PLD is broken (why?)
	//_logf("alloc_ack len%i pl %i %i", responseLength, responseBuffer[0], responseBuffer[1]);
	//RFM73_WriteAckPayload(responseBuffer, responseLength);
}



/*
streamWrapper * rfopen(uint32_t remoteAddress, char mode){
	
}
*/