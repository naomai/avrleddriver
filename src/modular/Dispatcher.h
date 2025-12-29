/*
 * Dispatcher.h
 *
 * Created: 27.12.2019 14:10:42
 *  Author: bun
 */ 


#ifndef DISPATCHER_H_
#define DISPATCHER_H_


#include "Module.h"

#define DISPATCHER_MAX_MODULES 9

typedef struct{
	uint8_t counter;
	Module * module;
} dispatcherModuleDescriptor;

class Dispatcher{
	protected:
	dispatcherModuleDescriptor modules[DISPATCHER_MAX_MODULES];
	void dispatchEvent(eventDescriptor *ed);
	public:
	EventQueue *event_queue;
	Dispatcher();
	~Dispatcher();
	void lateInitAll();
	void tick();
	void registerModule(Module * m);
	void unregisterModule(Module * m);
};


#endif /* DISPATCHER_H_ */