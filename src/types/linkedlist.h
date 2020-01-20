/*
 * linkedlist.h
 *
 * Created: 22.12.2019 22:42:28
 *  Author: bun
 */ 


#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct s_LinkedListNode{
	struct s_LinkedListNode *nextNodePtr;
	void *dataPtr;
	
} LinkedListNode;

class LinkedList{
	protected:
	uint8_t length;
	LinkedListNode *first, *last;
	LinkedListNode *iterCurrent;

	public:
	LinkedList();
	~LinkedList();
	void add(void *ptr);
	void remove(void *ptr);
	void clearList();
	void* current();
	void* next();
	void* reset();
	bool isValid();
};




#endif /* LINKEDLIST_H_ */