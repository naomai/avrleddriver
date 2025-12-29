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

#ifdef __cplusplus

#define EVENT_QUEUE_SIZE 32

class EventQueue{
	protected:
	eventDescriptor queue[EVENT_QUEUE_SIZE];
	uint8_t offsetOut, offsetIn;
	
	public:
	EventQueue();
	void pushEvent(uint8_t type,uint8_t lbyte,uint8_t hbyte,uint8_t source=0);
	void pushEvent(eventDescriptor *ev);
	eventDescriptor* popEvent();
	bool isEmpty();
	void clear();
	
};

#endif

#endif /* EVENTQUEUE_H_ */