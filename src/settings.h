/*
 * eemprom.h
 *
 * Created: 08.11.2019 16:59:42
 *  Author: bun
 */ 


#ifndef EEMPROM_H_
#define EEMPROM_H_

#include "config.h"
#include "types.h"
#include "io/multistream.h"

uint8_t loadSettings();
void updateSettings();
uint8_t readSettingsFromStream(streamWrapper *stream);
void writeSettingsToStream(streamWrapper *stream);

typedef enum {
	ec_StreamEnd = 0xFF,
	ec_HSV = 0x01,
	ec_Special = 0x02,
	ec_Entropy = 0x03,
	ec_RGB = 0x04, 
	// opcodes for remote control, ignored when reading from eeprom
	ec_Anim = 0x30, 
	ec_Mode = 0x31,
	ec_EncoderAction = 0x32,
	ec_SaveSettingsToStream = 0x33
} dataChunk;

#endif /* EEMPROM_H_ */