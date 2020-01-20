/*
 * CFile1.c
 *
 * Created: 13.11.2019 17:13:24
 *  Author: bun
 */ 
/*
#include "mcu.h"
#include "i2c.h"
#include "debug.h"

#define I2C_FREQ 100000UL
#define I2C_BITRATE_REG F_CPU/2/I2C_FREQ - 8
#define I2C_MY_ADDRESS 0x61

void initI2C();
void i2cSlaveState(uint8_t state);
void i2cAck(uint8_t ack);
void i2cStart();
void i2cStop();
void i2cAddress(uint8_t addr, flagRw rw);
void i2cSendByte(uint8_t byte);
uint8_t i2cReadByte();
void i2cWaitReady();
uint8_t i2cGetState();
void i2cSolveUnexpectedState() __attribute__((optimize("Os")));

void initI2C(){
	if(!(TWCR & (1<<TWEN))){
		TWBR = I2C_BITRATE_REG;
		TWCR = (1<<TWEN) | (1<<TWINT);
		TWAR = (I2C_MY_ADDRESS<<1);
	}
	//i2cWaitReady();
}

void i2cSlaveState(uint8_t state){
	TWCR |= (state<<TWEA);
}

void i2cAck(uint8_t ack){
	TWCR |= (ack<<TWEA);
}

void i2cStart(){
	TWCR |= (1 << TWSTA) | (1<<TWINT);
	i2cWaitReady();
}

void i2cStop(){
	TWCR |= (1 << TWSTO) | (1<<TWINT);
	i2cWaitReady();
}

void i2cAddress(uint8_t addr, flagRw rw){
	uint8_t realAddr = (addr << 1) | rw;
	i2cSendByte(realAddr);
}

void i2cSendByte(uint8_t byte){
	//TWCR &= ~(1 << TWSTA);
	i2cState state = i2cGetState();
	if(state == I2C_STATE_SLAR_ADDR || state==I2C_STATE_SLAR_BSENT_ACK){
		TWDR = byte;
		TWCR |= (1 << TWINT);
		i2cWaitReady();
	}else{
		i2cSolveUnexpectedState();
	}
	
}

uint8_t i2cReadByte(){
	i2cState state = i2cGetState();
	//_logf("ReadByte state=%x", state);
	if(state == I2C_STATE_SLAW_ADDR || state==I2C_STATE_SLAW_BRECV_ACK){
		TWCR |= (1 << TWINT);
		i2cWaitReady();
		//_logf("ReadByte byte=%x", TWDR);
		//_logf("ReadByte state=%x", i2cGetState());
		return TWDR;
	}else{
		i2cSolveUnexpectedState();
		return 255;
	}
}

void i2cSolveUnexpectedState(){
	i2cState state = i2cGetState();
	if(state == I2C_STATE_SLAW_BRECV_NACK || state == I2C_STATE_SLAW_STOP ||
		state == I2C_STATE_SLAR_BSENT_NACK || state == I2C_STATE_SLAR_BSENT_LAST){
		i2cAck(0);
		TWCR |= (1 << TWINT);
		i2cWaitReady();
	}else{
		TWCR &= ~(1<<TWEN);
		initI2C();
	}
}

void i2cWaitReady(){
	while(!(TWCR & (1 << TWINT)));
}


uint8_t i2cGetState(){
	return TWSR & 0xF8;
}

i2cRequestType i2cGetPendingRequest(){
	switch(i2cGetState()){
		case I2C_STATE_SLAW_ADDR:
			return i2cReq_SlaW;
		case I2C_STATE_SLAR_ADDR:
			return i2cReq_SlaR;
		default:
			return i2cReq_None;
	}
}

// seq
streamWrapper* i2copen(uint8_t address, char mode){
	i2cStream *stream;
	stream = malloc(sizeof(i2cStream));
	stream->mode = mode;
	stream->address = 0; //address;
	initI2C();

	//if(address==0){
		i2cSlaveState(1);
	//}

	
	streamWrapper *wrapper = wrapStream(stream,STREAM_I2C);

	wrapper->_close = i2cclose;
	wrapper->_read = i2cread;
	wrapper->_getc = i2cgetc;
	wrapper->_getdw = i2cgetdw;
	wrapper->_write = i2cwrite;
	wrapper->_putc = i2cputc;
	wrapper->_putdw = i2cputdw;
	wrapper->_eof = i2ceof;

	return wrapper;
}

void i2cclose(i2cStream *stream){
	i2cAck(0);
	i2cState state=i2cGetState();
	if(state==I2C_STATE_SLAR_ADDR || state==I2C_STATE_SLAR_BSENT_ACK){
		i2cputc(0xFF, stream);
	}else if(state==I2C_STATE_SLAW_ADDR || state==I2C_STATE_SLAW_BRECV_ACK){
		i2cgetc(stream);
	}
	_log("I2C CLOSE");
	TWCR &= ~(1<<TWEN);
	free(stream);
	initI2C();
}

uint8_t i2cgetc(i2cStream *stream){
	//i2cWaitReady();
	return i2cReadByte(1);
}

uint32_t i2cgetdw(i2cStream *stream){
	uint32_t result;
	//i2cWaitReady();
	//i2cAck(1);
	result  = (uint32_t)i2cReadByte() << 24;
	result |= (uint32_t)i2cReadByte() << 16;
	result |= (uint32_t)i2cReadByte() << 8;
	result |= (uint32_t)i2cReadByte();
	return result;
}

uint8_t i2cread(char *ptr, size_t size, size_t nitems, i2cStream *stream){
	uint16_t bytes = nitems*size;
	for(int i = 0; i<bytes; i++){
		//i2cAck(1);
		ptr[i] = i2cReadByte(1);
	}
	return 0;
}

void i2cputc(uint8_t byte, i2cStream *stream){

	i2cSendByte(byte);

}

void i2cputdw(uint32_t value, i2cStream *stream){

	i2cSendByte((value >> 24)  &0xFF);
	i2cSendByte((value >> 16) & 0xFF);
	i2cSendByte((value >> 8) & 0xFF);
	i2cSendByte(value & 0xFF);

}

uint8_t i2cwrite(char *ptr, size_t size, size_t nitems, i2cStream *stream){
	uint16_t bytes = nitems*size;

	for(int i = 0; i<bytes; i++){

		i2cSendByte(ptr[i]);

	}
		
	return 0;
}

uint8_t i2ceof(i2cStream *stream){
	if(!(TWCR & (1 << TWINT))) // no action pending
		return 1;
		
	i2cState state = i2cGetState();
	if(state == I2C_STATE_SLAW_BRECV_ACK || 
	   state==I2C_STATE_SLAW_ADDR) // master has more for us
		return 0;

	return 1;
}
*/


/* note to self: from main loop

#ifdef I2C_ENABLE_REMOTE
i2cRequestType req;
req=i2cGetPendingRequest();
if(req == i2cReq_SlaW){
	_log("I2C REQ W");
	readSettingsFromStream(remoteStream);
	_log("I2C REQ END");
	sclose(remoteStream);
	remoteStream = i2copen(0, 'r');
}
#endif

*/

