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
	LinkedListNode *newNode = (LinkedListNode*)malloc(sizeof(LinkedListNode));
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
	LinkedListNode *current=this->first, *next, *previous = NULL;
	for(uint8_t i=0; i<this->length; i++){
		next = current->nextNodePtr;
		if(current->dataPtr == ptr){
			if(i==0){
				this->first = next;
			}else{
				if(i == this->length-1){
					this->last = previous;
				}
				previous->nextNodePtr = next;
			}
			free(current);
			this->length--;
			break;
		}
		previous = current;
		current = next;
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


