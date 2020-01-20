/*
 * EventQueue.cpp
 *
 * Created: 27.12.2019 13:33:52
 *  Author: bun
 */ 

#include "EventQueue.h"

EventQueue::EventQueue(){
	this->clear();
}



eventDescriptor EventQueue::popEvent(){
	eventDescriptor ev;
	if(!this->isEmpty()){
		ev = queue[offsetOut];
		offsetOut++;
		if(offsetOut >= EVENT_QUEUE_SIZE) offsetOut=0;
	}
	return ev;
}

void EventQueue::pushEvent(eventDescriptor ev){
	queue[offsetIn] = ev;
	offsetIn++;
	if(offsetIn >= EVENT_QUEUE_SIZE) offsetIn=0;
}

void EventQueue::clear(){
	offsetOut = offsetIn = 0;
}

bool EventQueue::isEmpty(){
	return offsetOut == offsetIn;
}
	
