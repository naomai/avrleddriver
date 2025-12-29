/*
 * spi_extension_master.h
 *
 * Created: 14.02.2020 13:36:01
 *  Author: bun
 */ 


#ifndef SPI_EXTENSION_MASTER_H_
#define SPI_EXTENSION_MASTER_H_

#include "../modular/EventQueue.h"

typedef enum{
	SPIEX_PACKET_NOP = 0x00, // master poll / queue empty
	SPIEX_PACKET_EVENT = 0x01, // event dequeued
	SPIEX_PACKET_SLEEPMODE = 0x02, // place bus in sleep mode
	SPIEX_PACKET_ECHO = 0x10, // echo request
	SPIEX_PACKET_ECHO_RESPONSE = 0x11, // request.eventInfo = response.eventInfo
} spiexPacketType;

typedef struct {
	spiexPacketType type;
	eventDescriptor eventInfo;
} spiexPacket;


#ifdef __cplusplus
extern "C" {
#endif

void spiex_init();
void spiex_exchange(spiexPacket *packet);
void spiex_sleep();
void spiex_wakeup();
bool spiex_busy();

#ifdef __cplusplus
}
#endif

#endif /* SPI_EXTENSION_MASTER_H_ */