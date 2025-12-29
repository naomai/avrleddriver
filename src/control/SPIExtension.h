/*
 * SPIExtension.h
 *
 * Created: 14.02.2020 14:44:37
 *  Author: bun
 */ 


#ifndef SPIEXTENSION_H_
#define SPIEXTENSION_H_

#include <stdbool.h>
#include "../modular/Module.h"

class SPIExtension : public Module{
	public:
	SPIExtension();
	~SPIExtension();
	
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void tick();
	
	protected:
	EventQueue *spiexQueue;
};

#endif /* SPIEXTENSION_H_ */