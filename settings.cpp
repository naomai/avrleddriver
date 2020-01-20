/*
 * eeprom.c
 *
 * Created: 08.11.2019 16:59:33
 *  Author: bun
 */ 

#include "mcu.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include "io/eestream.h"
#include "settings.h"
#include "types.h"
#include "extender.h"
#include "debug.h"
#include "io/multistream.h"
#include "random.h"
#include "display/animation.h"
#include "LedDriver.h"

extern LedDriver *lights;


//#pragma GCC optimize("Os")
uint8_t loadSettings() {
	streamWrapper *stream = esopen(EEPROM_CONFIG_LOC, 'r');
	//_logf("LoadSettings stream size: %d", stream->size);
	uint8_t result = readSettingsFromStream(stream);
	sclose(stream);
	return result;
}

void updateSettings(){
	cli();
	streamWrapper *stream = esopen(EEPROM_CONFIG_LOC, 'w');
	writeSettingsToStream(stream);
	sclose(stream);
	sei();
}


uint8_t readSettingsFromStream(streamWrapper *stream) {
	dataChunk chunkType;
	uint8_t recordsIn = 0;
	uint8_t stripId;
	uint8_t streamEnd = 0;
	
	//_logf("LoadSettings stream type: %d", stream->type);
	
	do{
		if(seof(stream)){
			break;
		}
		chunkType = (dataChunk)sgetc(stream);
		//_logf("ChunkType=%d",chunkType);
		colorHSV newColorH;
		colorRaw newColorR;
		streamWrapper *altStream;
		LedLight *s;
		switch(chunkType){
			case ec_HSV:
				stripId = sgetc(stream);
				sread(&newColorH,sizeof(newColorH),1,stream);
				if(stripId>=STRIPS_COUNT) break;
				s=lights->getLightById(stripId);
				s->setColor(HSV2RGB(newColorH), LIGHT_COLOR_USER, COLORSPACE_SRGB);
				s->special &= 0xF0;
				break;
			case ec_RGB:
				stripId = sgetc(stream);
				sread(&newColorR,sizeof(newColorR),1,stream);
				if(stripId>=STRIPS_COUNT) break;
				s=lights->getLightById(stripId);
				s->setColor(newColorR, LIGHT_COLOR_USER, COLORSPACE_RAW);
				s->special &= 0xF0;
				break;
			case ec_Special:
				stripId = sgetc(stream);
				if(stripId>=STRIPS_COUNT) break;
				s=lights->getLightById(stripId);
				s->special = sgetc(stream) & 0x0F;
				s->applySpecialColor();
				
				break;	
			case ec_Entropy:
				randomFeedEntropyDword(sgetc(stream));
				//_logf("LoadSettings entropy: %lx", settings->entropy);
				break;
			case ec_Anim:
			{
				stripId = sgetc(stream);
				animation * anim;
				uint8_t speed;
				s = lights->getLightById(stripId);
				sread(&newColorR, sizeof(newColorR), 1, stream);
				
				speed = sgetc(stream); //newColorR.rgb.special;
				//newColorR.rgb.special = 0;
				if(stripId>=STRIPS_COUNT) break;
				anim = animCreate(stripId, s->getColor(LIGHT_COLOR_DISPLAY), newColorR, speed << 8, ANIM_REMOTE);
				
				animStart(anim);
				break;
			}
			case ec_SaveSettingsToStream:
				if(stream->type == STREAM_EEPROM){
					sgetc(stream);
					break;
				}
				switch(sgetc(stream)){
					case STREAM_EEPROM:
						altStream = esopen(EEPROM_CONFIG_LOC, 'w');
						writeSettingsToStream(altStream);
						sclose(altStream);
						break;
					case STREAM_I2C:
						#ifdef I2C_ENABLE_REMOTE
						/*sclose(stream);
						streamEnd = 1;
						stream = i2copen(0, 'w');*/
						writeSettingsToStream(stream);
						#endif
						break;
				}
				break;
			default:
				streamEnd = 1;
				recordsIn--;
				break;
		}
		recordsIn++;
		
	}while(!streamEnd);
	
	_logf("LoadSettings recordsIn: %d", recordsIn);
	
	return recordsIn;
}

void writeSettingsToStream(streamWrapper *stream){
	for(uint8_t i=0; i<STRIPS_COUNT; i++){
		LedLight *strip = lights->getLightById(i);
		if(strip->special & 0x1F){ // special color
			sputc(ec_Special, stream);
			sputc(i, stream);
			sputc(strip->special & 0x1F, stream);
			_logf("writeSpecial: %i %i", i, strip->special);
		}else{ // regular rgb
			sputc(ec_RGB, stream);
			sputc(i, stream);
			colorRaw rgb = strip->getColor(LIGHT_COLOR_SET, COLORSPACE_RAW);
			swrite(&rgb, sizeof(colorRaw), 1, stream);
			_logf("writeRgb: %i", i);
		}
		/*animation * anim;
		anim = animCreate(i, colorWhite, strip->getColor(LIGHT_COLOR_USER), 8 << 8, ANIM_TEMPFX);
		animStart(anim);*/
	}
	if(stream->type != STREAM_EEPROM){
		//sputc(ec_Mode, stream);
	}
	sputc(ec_Entropy, stream);
	sputdw(random32(), stream);
	sputc(ec_StreamEnd, stream);
	
}
