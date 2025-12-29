/*
 * SensorHub.cpp
 *
 * Created: 12.04.2022 17:43:26
 *  Author: bun
 */ 
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "SensorHub.h"
#include "../PowerMgmt.h"


SensorHub::SensorHub() : Module(){
	SensorData * s;
	for(uint8_t i=0; i<ENTITY_COUNT; i++){
		s = (SensorData*)calloc(1, sizeof(SensorData));
		memset(s, 0, MAX_BYTES_PER_ENTITY);
		s->type = (entityType)0;
		this->sensor_data[i] = s;		
	}
	powerDown = false;

}


SensorHub::~SensorHub(){
	for(uint8_t i=0; i<ENTITY_COUNT; i++){
		free(this->sensor_data[i]);
	}
}

void SensorHub::lateInit(){

}


void SensorHub::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	uint16_t frameId = (hbyte<<8) | lbyte;
	if(type==EVENT_FRAME){
		
	}else if(type==EVENT_DEEP_SLEEP_REQUESTED){

	}else if(type==EVENT_POWERSTATE){

	}
}

SensorData* SensorHub::getSensorById(uint8_t sensorId){
	if(sensorId >= ENTITY_COUNT) return NULL;
	return this->sensor_data[sensorId];
}
