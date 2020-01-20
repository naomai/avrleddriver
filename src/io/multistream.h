/*
 * multistream.h
 *
 * Created: 13.11.2019 18:43:47
 *  Author: bun
 */ 


#ifndef MULTISTREAM_H_
#define MULTISTREAM_H_

#ifdef __cplusplus
extern "C" {
	#endif


#include <stdio.h>
#include <stdlib.h>

typedef enum{
	STREAM_EEPROM, 
	STREAM_I2C
} streamWrapperType;

typedef struct {
	streamWrapperType type;
	void *stream;
	uint8_t (*_read)();
	uint8_t (*_getc)();
	uint32_t (*_getdw)();
	uint8_t (*_write)();
	void (*_putc)();
	void (*_putdw)();
	void (*_close)();
	uint8_t (*_eof)();
}streamWrapper;

streamWrapper *wrapStream(void* stream, uint8_t type);
void sclose(streamWrapper *stream);
void sread(void *ptr, size_t size, size_t nitems, streamWrapper *stream);
uint8_t sgetc(streamWrapper *stream);
uint32_t sgetdw(streamWrapper *stream);
void swrite(void *ptr, size_t size, size_t nitems, streamWrapper *stream);
void sputc(uint8_t byte, streamWrapper *stream);
void sputdw(uint32_t value, streamWrapper *stream);
uint8_t seof(streamWrapper *stream);

	#ifdef __cplusplus
}
#endif

#endif /* MULTISTREAM_H_ */