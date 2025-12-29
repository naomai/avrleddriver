/*
 * SensorHub.cpp
 *
 * Created: 12.04.2022 17:43:26
 *  Author: bun
 */ 
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "DHT.h"
#include "../PowerMgmt.h"
#include "../hardware/sensor/dht.h"

extern SensorHub *sensors;

DHT::DHT() : Module(){
	
}


DHT::~DHT(){
	
}

void DHT::lateInit(){
	temperatureSensor=sensors->getSensorById(7);
	humiditySensor=sensors->getSensorById(8);
	temperatureSensor->type = SENSOR_TEMPERATURE;
	humiditySensor->type = SENSOR_HUMIDITY;
}


void DHT::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	uint16_t frameId = (hbyte<<8) | lbyte;
	if(type==EVENT_FRAME && frameId % 30 == 0){
		float t = 0, h=0;
		uint32_t ti, hi;

		if(dht_gettemperaturehumidity(&t, &h) != -1) {
			if(havg == 0){
				tavg = t;
				havg = h;
			}else{
				tavg = (tavg*0.75 + t*0.25);
				havg = (havg*0.95 + h*0.05);
			}

			ti = round(100.0 * tavg);
			hi = round(100.0 * havg);

			memcpy(temperatureSensor->data, &ti, sizeof(ti));
			memcpy(humiditySensor->data, &hi, sizeof(hi));
		}
	}else if(type==EVENT_DEEP_SLEEP_REQUESTED){

	}else if(type==EVENT_POWERSTATE){

	}
}
