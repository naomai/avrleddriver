/*
 * SensorHub.h
 *
 * Created: 12.04.2022 17:46:25
 *  Author: bun
 */ 


#ifndef SENSORDHT_H_
#define SENSORDHT_H_

#include <stdint.h>
#include <stdbool.h>
#include "../config.h"
#include "../modular/Module.h"
#include "../display/LightConfig.h"
#include "SensorHub.h"



class DHT : public Module{
	protected:
	SensorData *temperatureSensor;
	SensorData *humiditySensor;
	bool powerDown;
	float tavg = 0, havg = 0;
	
	public:
	DHT();
	~DHT();
	
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void lateInit();
};



#endif /* SENSORDHT_H_ */