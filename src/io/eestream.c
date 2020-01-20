/*
 * eestream.c
 *
 * Created: 10.11.2019 21:08:59
 *  Author: bun
 */ 

#include <avr/eeprom.h>
#include <stdlib.h>
#include <stdio.h>
#include "eestream.h"
#include "multistream.h"

#pragma GCC optimize("Os")

// eeprom sequential reader

streamWrapper* esopen(void *startOffset, char mode){
	eepromStream *stream;
	stream = calloc(sizeof(eepromStream),1);
	stream->mode = mode;
	stream->ptr = startOffset;
	stream->dataptr = stream->ptr + 2;
	if(mode=='w'){
		stream->size = 0;
	}else{
		stream->size = eeprom_read_word(startOffset);
	}
	
	if(mode=='r' || mode=='w'){
		stream->current = stream->dataptr;
		}else if(mode=='a'){
		esseek(stream, 0, SEEK_END);
	}
	streamWrapper *wrapper = wrapStream(stream,STREAM_EEPROM);
	wrapper->_close = esclose;
	wrapper->_read = esread;
	wrapper->_getc = esgetc;
	wrapper->_getdw = esgetdw;
	wrapper->_write = eswrite;
	wrapper->_putc = esputc;
	wrapper->_putdw = esputdw;
	wrapper->_eof = eseof;
	return wrapper;
}
void esclose(eepromStream *stream){
	if(stream->mode != 'r'){
		eeprom_update_word(stream->ptr, stream->size);
	}
	free(stream);
}

// read
uint8_t esread(void *ptr, size_t size, size_t nitems, eepromStream *stream){
	for(size_t i = 0; i<nitems; i++){
		eeprom_read_block(ptr+i*size,stream->current,size);
		stream->current += size;
	}
	return 0; // todo
}
uint8_t esgetc(eepromStream *stream){
	return eeprom_read_byte(stream->current++);
}
uint32_t esgetdw(eepromStream *stream){
	uint32_t result = eeprom_read_dword(stream->current);
	stream->current += 4;
	return result;
}

//write
uint8_t eswrite(void *ptr, size_t size, size_t nitems, eepromStream *stream){
	if(stream->mode == 'r') return -1;
	for(size_t i = 0; i<nitems; i++){
		eeprom_update_block(ptr+i*size,stream->current,size);
		stream->current += size;
	}
	__esupdatesize(stream);
	return 0; //todo
}
void esputc(uint8_t byte, eepromStream *stream){
	if(stream->mode == 'r') return;
	eeprom_update_byte(stream->current++, byte);
	__esupdatesize(stream);
}
void esputdw(uint32_t value, eepromStream *stream){
	if(stream->mode == 'r') return;
	eeprom_update_dword(stream->current, value);
	stream->current+=4;
	__esupdatesize(stream);
}

// misc
size_t estell(eepromStream *stream){
	return stream->current - stream->dataptr;
}
int8_t esseek(eepromStream *stream, int16_t offset, int origin){
	void *tempptr;
	switch(origin){
		case SEEK_SET:
		if(offset < 0) return -1;
		stream->current = stream->dataptr + offset;
		break;
		case SEEK_CUR:
		tempptr = stream->current + offset;
		if(tempptr < stream->dataptr || tempptr > stream->dataptr + stream->size){
			return -1;
		}
		stream->current=tempptr;
		break;
		case SEEK_END:
		stream->current = stream->dataptr + stream->size + offset;
		break;
	}
	return 0;
}

uint8_t eseof(eepromStream *stream){
	return estell(stream) >= stream->size;
}

void __esupdatesize(eepromStream *stream){
	size_t pos;
	if((pos=estell(stream)) > stream->size){
		stream->size = pos;
	}
}