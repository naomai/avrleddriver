/*
 * i2c.h
 *
 * Created: 13.11.2019 18:03:12
 *  Author: bun
 */ 


#ifndef I2C_H_
#define I2C_H_

#ifdef __cplusplus
extern "C" {
	#endif


#include <stdio.h>
#include <stdint.h>
#include "multistream.h"

typedef enum {
	rw_Write = 0,
	rw_Read	= 1
} flagRw;

typedef struct{
	uint8_t address;
	char mode;
} i2cStream;


typedef enum{
	I2C_STATE_SLAW_ADDR = 0x60,
	I2C_STATE_SLAW_BRECV_ACK = 0x80,
	I2C_STATE_SLAW_BRECV_NACK = 0x88,
	I2C_STATE_SLAW_STOP = 0xA0,
	
	I2C_STATE_SLAR_ADDR = 0xA8,
	I2C_STATE_SLAR_BSENT_ACK = 0xB8,
	I2C_STATE_SLAR_BSENT_NACK = 0xC0,
	I2C_STATE_SLAR_BSENT_LAST = 0xC8
}i2cState;

typedef enum {
	i2cReq_None,
	i2cReq_SlaW,
	i2cReq_SlaR	
}i2cRequestType;

streamWrapper* i2copen(uint8_t address, char mode);
void i2cclose(i2cStream *stream);
uint8_t i2cgetc(i2cStream *stream);
uint32_t i2cgetdw(i2cStream *stream) __attribute__((optimize("Os")));
uint8_t i2cread(char *ptr, size_t size, size_t nitems, i2cStream *stream);
void i2cputc(uint8_t byte, i2cStream *stream);
void i2cputdw(uint32_t value, i2cStream *stream) __attribute__((optimize("Os")));
uint8_t i2cwrite(char *ptr, size_t size, size_t nitems, i2cStream *stream);
uint8_t i2ceof(i2cStream *stream);
i2cRequestType i2cGetPendingRequest();

	#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */