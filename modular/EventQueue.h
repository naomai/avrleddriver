/*
 * EventQueue.h
 *
 * Created: 27.12.2019 13:57:22
 *  Author: bun
 */ 


#ifndef EVENTQUEUE_H_
#define EVENTQUEUE_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct{
	uint8_t source;
	uint8_t type;
	uint8_t lbyte;
	uint8_t hbyte;
} eventDescriptor;

#define EVENT_QUEUE_SIZE 16

class EventQueue{
	friend class Dispatcher;
	protected:
	eventDescriptor queue[EVENT_QUEUE_SIZE];
	uint8_t offsetOut, offsetIn;
	
	eventDescriptor popEvent();
	bool isEmpty();

	public:
	EventQueue();
	void pushEvent(eventDescriptor ev);
	void clear();
	
};


#endif /* EVENTQUEUE_H_ */