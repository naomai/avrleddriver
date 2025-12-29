/*
 * Module.cpp
 *
 * Created: 27.12.2019 12:54:51
 *  Author: bun
 */ 

#include "Module.h"

uint8_t Module::moduleIdCounter = 1;

Module::Module(){
	this->tickPrescaler = 20;
	this->moduleId = Module::moduleIdCounter++;
	lateInitDone = false;
}

void Module::raiseEvent(uint8_t type, uint8_t lbyte, uint8_t hbyte, bool loopback){
	this->eq->pushEvent(type, lbyte, hbyte,  !loopback ? this->moduleId : 0);
}

void Module::lateInit(){

}

void Module::tick(){
	
}

void Module::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	
}
