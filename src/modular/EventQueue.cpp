/*
 * EventQueue.cpp
 *
 * Created: 27.12.2019 13:33:52
 *  Author: bun
 */ 

#include "EventQueue.h"
#include <string.h>

EventQueue::EventQueue(){
	this->clear();
}



eventDescriptor* EventQueue::popEvent(){
	eventDescriptor *ev;
	if(!this->isEmpty()){
		ev = &this->queue[offsetOut];
		offsetOut++;
		if(offsetOut >= EVENT_QUEUE_SIZE) offsetOut=0;
	}
	return ev;
}

void EventQueue::pushEvent(uint8_t type,uint8_t lbyte,uint8_t hbyte,uint8_t source){
	eventDescriptor ev;
	ev.type = type;
	ev.lbyte = lbyte;
	ev.hbyte = hbyte;
	ev.source = source;
	pushEvent(&ev);
}

void EventQueue::pushEvent(eventDescriptor *ev){
	memcpy(&this->queue[offsetIn], ev, sizeof(*ev));
	this->offsetIn++;
	if(this->offsetIn >= EVENT_QUEUE_SIZE) this->offsetIn=0;
}

void EventQueue::clear(){
	offsetOut = offsetIn = 0;
}

bool EventQueue::isEmpty(){
	return offsetOut == offsetIn;
}
	
