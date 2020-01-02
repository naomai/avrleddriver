/*
 * Module.h
 *
 * Created: 27.12.2019 12:55:37
 *  Author: bun
 */ 


#ifndef MODULE_H_
#define MODULE_H_

#include <stdint.h>
#include <stdbool.h>

// builtin
#define EVENT_FRAME 0x01
#define EVENT_COLOR_CHANGE 0x02

#include "EventQueue.h"

class Module{
	friend class Dispatcher;
	private:
	EventQueue *eq;
	uint8_t moduleId;
	bool lateInitDone;
	static uint8_t moduleIdCounter;
	
	protected:
	uint8_t tickPrescaler;
	void raiseEvent(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	
	public: 
	Module();
	virtual void lateInit();
	virtual void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	virtual void tick();
};

#include "Dispatcher.h"

#endif /* MODULE_H_ */