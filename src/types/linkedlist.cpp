/*
 * simplelist.cpp
 *
 * Created: 22.12.2019 21:46:33
 *  Author: bun
 */ 

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "linkedlist.h"

LinkedList::LinkedList(){
	
}

LinkedList::~LinkedList(){
	this->clearList();
}

	
void LinkedList::add(void *ptr){
	LinkedListNode *newNode = new LinkedListNode;
	newNode->dataPtr = ptr;
	newNode->nextNodePtr = NULL;
	if(!this->length){
		this->first = newNode;
	}else{
		this->last->nextNodePtr = newNode;
	}
	this->last = newNode;
	this->length++;
}
	
void LinkedList::remove(void *ptr){
	LinkedListNode *curr=this->first, *next_, *previous_ = NULL;
	for(uint8_t i=0; i<this->length; i++){
		next_ = curr->nextNodePtr;
		if(curr->dataPtr == ptr){
			if(i==0){
				this->first = next_;
			}else{
				if(i == this->length-1){
					this->last = previous_;
				}
				previous_->nextNodePtr = next_;
			}
			delete curr;
			//free(current);
			this->length--;
			break;
		}
		previous_ = curr;
		curr = next_;
	}
}
	


void LinkedList::clearList(){
	LinkedListNode *current=this->first, *next;
	for(uint8_t i=0; i<this->length; i++){
		next = current->nextNodePtr;
		free(current);
		current=next;
	}
}
	
void* LinkedList::current(){
	return iterCurrent->dataPtr;
}	
	
void* LinkedList::next(){
	if(this->isValid()){
		this->iterCurrent = this->iterCurrent->nextNodePtr;
	}
	return this->current();
}
void* LinkedList::reset(){
	this->iterCurrent = this->first;
	return this->current();
}
bool LinkedList::isValid(){
	return this->iterCurrent != NULL;
}


