/*
 * EncoderModule.h
 *
 * Created: 27.12.2019 14:50:19
 *  Author: bun
 */ 


#ifndef ENCODERMODULE_H_
#define ENCODERMODULE_H_

#include <stdbool.h>
#include "../modular/Module.h"

typedef enum {
	ENC_NONE, ENC_CW, ENC_CCW, ENC_BUTDOWN, ENC_BUTUP
} encoderEvent;

#define EVENT_INPUT_KEY 0x10
#define EVENT_INPUT_DELTA 0x11

#define KEY_ENTER 13
#define KEY_LEFT 37
#define KEY_RIGHT 39

#define KEYSTATE_DOWN 0x01
#define KEYSTATE_UP 0x00


class Encoder : public Module{
	protected:
	char lastEncoderState0,lastEncoderState1;
	char lastButState;
	bool state0changed;
	encoderEvent checkEncoder();
	
	public:
	Encoder();
	virtual void tick();
	bool checkEncoderButton();
};


#endif /* ENCODERMODULE_H_ */