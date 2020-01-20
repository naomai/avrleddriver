/*
 * multistream.c
 *
 * Created: 13.11.2019 18:43:20
 *  Author: bun
 */ 

#include "multistream.h"

streamWrapper *wrapStream(void* stream, uint8_t type){
	streamWrapper *newWrapper;
	newWrapper = calloc(sizeof(streamWrapper),1); 
	newWrapper->stream = stream;
	newWrapper->type = type;
	return newWrapper;
}

void sread(void *ptr, size_t size, size_t nitems, streamWrapper *stream){
	stream->_read(ptr,size,nitems,stream->stream);
}

uint8_t sgetc(streamWrapper *stream){
	return stream->_getc(stream->stream);
}
uint32_t sgetdw(streamWrapper *stream){
	return stream->_getdw(stream->stream);
}

void swrite(void *ptr, size_t size, size_t nitems, streamWrapper *stream){
	stream->_write(ptr,size,nitems,stream->stream);
}
void sputc(uint8_t byte, streamWrapper *stream){
	stream->_putc(byte,stream->stream);
}
void sputdw(uint32_t value, streamWrapper *stream){
	stream->_putdw(value,stream->stream);
}
void sclose(streamWrapper *stream){
	stream->_close(stream->stream);
	free(stream);
}

uint8_t seof(streamWrapper *stream){
	if(stream->_eof)
		return stream->_eof(stream->stream);
	return 0;
}