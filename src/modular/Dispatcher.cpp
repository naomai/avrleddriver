/*
 * Dispatcher.cpp
 *
 * Created: 27.12.2019 13:13:36
 *  Author: bun
 */ 

#include <stdint.h>
#include <stddef.h>
#include "Dispatcher.h"

Dispatcher::Dispatcher(){
	event_queue = new EventQueue;
}
Dispatcher::~Dispatcher(){
	dispatcherModuleDescriptor *md;
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		md = &modules[i];
		if(md->module == NULL) continue;
		md->module->eq = NULL;
	}
	delete event_queue;
}
void Dispatcher::lateInitAll(){
	dispatcherModuleDescriptor *md;
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		md = &modules[i];
		if(md->module == NULL) continue;
		
		if(!md->module->lateInitDone) {
			md->module->lateInit();
			md->module->lateInitDone = true;
		}
		
	}
}


void Dispatcher::tick(){
	eventDescriptor *currentEvent;
	while(!event_queue->isEmpty()){
		currentEvent = event_queue->popEvent();
		this->dispatchEvent(currentEvent);
	}
	
	dispatcherModuleDescriptor *md;
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		md = &modules[i];
		if(md->module == NULL) continue;
		
		if((md->counter++) >= md->module->tickPrescaler){
			md->module->tick();
			md->counter = 0;
		}
	}
		
}

void Dispatcher::dispatchEvent(eventDescriptor *ed){
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		Module *m = modules[i].module;
		if(m != NULL && m->moduleId != ed->source){
			m->event(ed->type, ed->lbyte, ed->hbyte);
			
		}
	}
}


void Dispatcher::registerModule(Module * m){
	dispatcherModuleDescriptor *md;
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		md = &modules[i];
		if(md->module == NULL){
			md->counter = 0;
			md->module = m;
			m->eq = event_queue;
			return;
		}
	}
}

void Dispatcher::unregisterModule(Module * m){
	dispatcherModuleDescriptor *md;
	for(uint8_t i = 0; i<DISPATCHER_MAX_MODULES; i++){
		md = &modules[i];
		if(md->module == m){
			md->module = NULL;
			m->eq = NULL;
			return;
		}
	}
}
