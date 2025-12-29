/*
 * SensorHub.h
 *
 * Created: 12.04.2022 17:46:25
 *  Author: bun
 */ 


#ifndef SENSORHUB_H_
#define SENSORHUB_H_

#include <stdint.h>
#include <stdbool.h>
#include "../config.h"
#include "../modular/Module.h"
#include "../display/LightConfig.h"
/*
typedef enum {
	SENSOR_HUMIDITY = 0x21,
	SENSOR_TEMPERATURE = 0x22,
	SENSOR_BATTERY = 0x23
} sensorType;*/

typedef struct {
	entityType type;
	//uint8_t entityId;
	uint8_t data[MAX_BYTES_PER_ENTITY];
} SensorData;

class SensorHub : public Module{
	protected:
	SensorData* sensor_data[ENTITY_COUNT];
	bool powerDown;
	
	public:
	SensorHub();
	~SensorHub();
	
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void lateInit();
	SensorData* getSensorById(uint8_t sensorId);
};



#endif /* SENSORHUB_H_ */