/*
 * SPIExtension.cpp
 *
 * Created: 14.02.2020 14:44:18
 *  Author: bun
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "SPIExtension.h"
#include "../hardware/spi_extension.h"
#include <string.h>

SPIExtension::SPIExtension(){
	spiex_init();
	spiexQueue = new EventQueue();
}
SPIExtension::~SPIExtension(){
	delete spiexQueue;
}

void SPIExtension::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	/*eventDescriptor ev;
	ev.type = type;
	ev.lbyte = lbyte;
	ev.hbyte = hbyte;*/
	spiexQueue->pushEvent(type, lbyte,hbyte);
}

void SPIExtension::tick(){
	spiexPacket packet;
	if(spiex_busy()) return;
	
	if(!spiexQueue->isEmpty()){
		packet.type = SPIEX_PACKET_EVENT;
		memcpy(&packet.eventInfo, spiexQueue->popEvent(), sizeof(eventDescriptor));
	}else{
		packet.type = SPIEX_PACKET_NOP;
	}
	spiex_exchange(&packet); // *exchange*
	switch(packet.type){
		case SPIEX_PACKET_EVENT:
			raiseEvent(packet.eventInfo.type, packet.eventInfo.lbyte, packet.eventInfo.hbyte);
			break;
		case SPIEX_PACKET_SLEEPMODE:
			spiex_sleep();
			break;
			
	}
}
