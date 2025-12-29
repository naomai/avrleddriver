/*
 * LedHardware.h
 *
 * Created: 22.12.2019 23:11:45
 *  Author: bun
 */ 


#ifndef LEDHARDWARE_H_
#define LEDHARDWARE_H_

#ifdef __cplusplus
	#include "../types/linkedlist.h"
	extern "C" {
#endif

#include "../display/LightConfig.h"
#include "../display/color.h"



typedef struct { // light state
	entityConfig hardwareConfig;
	colorRaw color; // currently displayed color
} light_s;

#ifdef __cplusplus
	}

	class LedHardware{
		protected:
		LinkedList *light_list;
		public:
		LedHardware();
		~LedHardware();
		light_s* registerLight(entityConfig *config);
		void initFinished();
		void tick();
	};
#endif

#endif /* LEDHARDWARE_H_ */