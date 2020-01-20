/*
 * SettingsModule.h
 *
 * Created: 27.12.2019 21:30:21
 *  Author: bun
 */ 


#ifndef SETTINGSMODULE_H_
#define SETTINGSMODULE_H_


#include <stdbool.h>
#include "modular/Module.h"


class Settings : public Module{
	protected:
	bool updateRequested;
	
	public:
	Settings();
	void lateInit();
	void tick();
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void saverBlinker();
	

};



#endif /* SETTINGSMODULE_H_ */