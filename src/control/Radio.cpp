/*
 * Radio.cpp
 *
 * Created: 28.12.2019 13:00:02
 *  Author: bun
 */ 

#include <string.h>
#include "../config.h"
#include "Radio.h"
#include "../hardware/RFM73.h"
#include "../random.h"
#include "../LedDriver.h"
#include "../debug.h"
#include "../display/animation.h"
#include "../PowerMgmt.h"

#define ADDR_PREFIX RFM73_ADDRESS_BASE
#define ADDR_MASTER (ADDR_PREFIX)


extern const lightConfig PROGMEM lightsConfig[STRIPS_COUNT];
extern LedDriver *lights;
extern PowerManagement *pwr;

Radio::Radio() : Module(){
	rfMode = RFMODE_SLEEP;
	myDeviceId = 1;
	responseLength = 0;
	waitFrames = FPS;
	ticksSinceLastComm = 0;
	wakesSinceLastComm = 0;
	responsePrepared = false;
	power = rfPowerState_On;	
	RFM73_Init();
	RFM73_SetChannel(RFM73_CHANNEL);
}


void Radio::frameTick(){
	uint8_t rfBuffer[PACKET_LEN];
	//bool status;
	ticksSinceLastComm++;
	
	if(--waitFrames){
		return;
	}
	waitFrames = 5;

	
	switch(rfMode){
		
		case RFMODE_ORPHAN:
			//greeting
			if(RFM73_IsRadioPresent()){
				PORTC ^= (1<<PC2);
				RFM73_InitChip();
				_log("Notice me sempai");
				responseBuffer[0] = rf_Register;
				responseBuffer[1] = myDeviceId;
				responseBuffer[2] = power;
				if(RFM73_Transmit(ADDR_MASTER, responseBuffer, 3)){
					//_log("Greeting sent");
					rfMode = RFMODE_REGISTERED;
					waitFrames = 2;
					ticksSinceLastComm = 0;
					wakesSinceLastComm = 0;
				}else{
					waitFrames = 2 + (random8() & 7); //avoid overlapping requests from two driver modules
				}
			}
			break;
		case RFMODE_REGISTERED:
			PORTC ^= (1<<PC2);
			if(RFM73_IsDataAvailable()){
				RFM73_ReadData(rfBuffer);
				rfMode = RFMODE_MASTER_WAITING;
				processCommand(rfBuffer);
				ticksSinceLastComm = 0;
				wakesSinceLastComm = 0;
			}
			if(ticksSinceLastComm > RF_TIMEOUT * FPS || wakesSinceLastComm >= 3){
				rfMode = RFMODE_ORPHAN;
			}
			waitFrames = 1;
			break;
		case RFMODE_READY_TO_RESPOND:
			RFM73_Transmit(ADDR_MASTER, responseBuffer, responseLength);
			rfMode = RFMODE_REGISTERED;
			responsePrepared = false;
			waitFrames = 2;
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

void Radio::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	if(type==EVENT_FRAME){
		frameTick();
	}else if(type==EVENT_COLOR_CHANGE && hbyte == LIGHT_COLOR_SET){
		reportState(lbyte);
	}else if(type==EVENT_DEEP_SLEEP_ENTER){
		power = rfPowerState_Sleep;
		reportEvent(rfEvent_PowerState, power, 0, false);
		RFM73_PowerDown();
	}else if(type==EVENT_DEEP_SLEEP_WAKEUP){
		RFM73_InitChip();
		RFM73_PowerUp();
		power = rfPowerState_On;
		reportEvent(rfEvent_PowerState, power);
	}else if(type==EVENT_DEEP_SLEEP_SHORTWAKE){
		RFM73_InitChip();
		RFM73_PowerUp();
		power = rfPowerState_ShortWake;
		reportEvent(rfEvent_PowerState, power);
		wakesSinceLastComm ++;
	}else if(type==EVENT_DEEP_SLEEP_REQUESTED){
		power = rfPowerState_ShortWake;
		reportEvent(rfEvent_PowerState, power);
	}
}

void Radio::tick(){
	 /*if(power != rfPowerState_On){
		 frameTick();
	 }*/
}


void Radio::signIn(uint8_t myId){
	myDeviceId = myId;
	rfMode = RFMODE_ORPHAN;
	RFM73_SetDeviceAddress(ADDR_PREFIX | myDeviceId);
}

void Radio::processCommand(uint8_t * data){
	rfCommand cmd = (rfCommand)data[0];
	switch(cmd){
		case rf_Request:
			//_logf("Req %i", data[1]);
			processRequest(data);
			break;
		case rf_ChangeState:
			//_logf("Req %i", data[1]);
			processChangeState(data);
			break;
		case rf_Check:
			if(responsePrepared){
				rfMode = RFMODE_READY_TO_RESPOND;
				waitFrames = 1;
			}
	}
}

void Radio::processRequest(uint8_t * data){
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
			sendResponse();
			break;
		case rfParam_State:
			respondState(data[2]);
			break;
	}
		
}

void Radio::processChangeState(uint8_t * data){
	rfParam reg = (rfParam)data[1];
	if(reg < 128 && reg < STRIPS_COUNT){
		rfRegisterSet setMode;
		LedLight* strip = lights->getLightById(reg);
		setMode = (rfRegisterSet)(data[3] & 0x0F);
		if(setMode == rfRegisterSet_Attribute){
			uint8_t special=data[2];
			strip->setSpecialAttribute(special);
		}else{
			uint8_t speed;
			speed = data[2];
			animation * anim;
			colorSpace cs;
			colorRaw newColor;
			if(setMode == rfRegisterSet_Reset){
				cs = COLORSPACE_RAW;
				newColor = strip->getColor(LIGHT_COLOR_SET, COLORSPACE_RAW);
			}else{
				cs = (colorSpace)((data[3]>>4) & 0x0F);
				memcpy(&newColor, &data[4], sizeof(colorRaw));
			
				if(cs==COLORSPACE_SRGB){
					newColor = strip->mapper->fromRGB(newColor);
				}
			}
			
			anim = animCreate(reg, strip->getColor(LIGHT_COLOR_DISPLAY, COLORSPACE_RAW), newColor, speed << 8, ANIM_REMOTE);
			animStart(anim);
			if(setMode == rfRegisterSet_Permanent){
				strip->setColor(newColor, LIGHT_COLOR_SET, COLORSPACE_RAW);
			}
		}
	}else if(reg==rfRegister_PowerState){
		if(power != data[2]){
			if(data[2]==rfPowerState_On){
				pwr->wakeup(true);
			}else if(data[2]==rfPowerState_Sleep){
				pwr->requestDeepSleep();
			}
		}
		
	}
}

void Radio::reportEvent(uint8_t radioEventType, uint8_t lbyte, uint8_t hbyte, bool queued){
	if(rfMode != RFMODE_ORPHAN && rfMode != RFMODE_SLEEP){
		responseBuffer[0] = rf_Event;
		responseBuffer[1] = myDeviceId;
		responseBuffer[2] = radioEventType;
		responseBuffer[3] = lbyte;
		responseBuffer[4] = hbyte;
		responseLength = 5;
		if(queued){
			rfMode=RFMODE_READY_TO_RESPOND;
			waitFrames = 1;
			responsePrepared = true;
		}else{
			RFM73_Transmit(ADDR_MASTER, responseBuffer, responseLength);
		}
	}
}


void Radio::reportState(uint8_t reg){
	responseBuffer[0] = rf_ChangeState;
	responseBuffer[1] = rfParam_State;
	respondState(reg);
}
	

void Radio::respondState(uint8_t reg){
	//_logf("STATERQ %i", reg);
	responseBuffer[2] = reg;
	if(reg < 128 && reg < STRIPS_COUNT){
		LedLight *s = lights->getLightById(reg);
		uint8_t bufferOffset = 3;
		colorRaw col = s->getColor(LIGHT_COLOR_USER, COLORSPACE_SRGB);
		memcpy(&responseBuffer[bufferOffset], &col, sizeof(col));
		bufferOffset += sizeof(col);
		col = s->getColor(LIGHT_COLOR_USER, COLORSPACE_RAW);
		memcpy(&responseBuffer[bufferOffset], &col, sizeof(col));
		bufferOffset += sizeof(col);
		responseBuffer[bufferOffset] = s->special & 0x0F;
		bufferOffset += 1;
		responseLength = bufferOffset;
		sendResponse();
	}else if(reg == rfRegister_PowerState){
		responseBuffer[3] = (uint8_t)power;
		responseLength=4;
		sendResponse();
	}
}

void Radio::sendResponse(){
	if(rfMode == RFMODE_MASTER_WAITING){
		rfMode=RFMODE_READY_TO_RESPOND;
		waitFrames = 1;
	}
	responsePrepared = true;
	// ACK+PLD is broken (why?)
	//_logf("alloc_ack len%i pl %i %i", responseLength, responseBuffer[0], responseBuffer[1]);
	//RFM73_WriteAckPayload(responseBuffer, responseLength);
}
