/*
 * EventQueue.cpp
 *
 * Created: 27.12.2019 13:33:52
 *  Author: bun
 */ 

#include "EventQueue.h"

EventQueue::EventQueue(){
	offsetOut = 0;
	offsetIn = 0;
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

bool EventQueue::isEmpty(){
	return offsetOut == offsetIn;
}
	
