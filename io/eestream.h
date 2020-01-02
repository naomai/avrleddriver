/*
 * eestream.h
 *
 * Created: 10.11.2019 21:11:38
 *  Author: bun
 */ 


#ifndef EESTREAM_H_
#define EESTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "multistream.h"

typedef struct{
	void *ptr;
	void *dataptr;
	void *current;
	char mode;
	size_t size;
} eepromStream;

streamWrapper* esopen(void *startOffset, char mode);
void esclose(eepromStream *stream);
uint8_t esread(void *ptr, size_t size, size_t nitems, eepromStream *stream);
uint8_t esgetc(eepromStream *stream);
uint32_t esgetdw(eepromStream *stream);
uint8_t eswrite(void *ptr, size_t size, size_t nitems, eepromStream *stream);
void esputc(uint8_t byte, eepromStream *stream);
void esputdw(uint32_t value, eepromStream *stream);
size_t estell(eepromStream *stream);
int8_t esseek(eepromStream *stream, int16_t offset, int origin);
uint8_t eseof(eepromStream *stream);
void __esupdatesize(eepromStream *stream);


#ifdef __cplusplus
}
#endif

#endif /* EESTREAM_H_ */