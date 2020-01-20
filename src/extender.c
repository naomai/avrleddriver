/*
 * CFile1.c
 *
 * Created: 12.11.2019 21:09:38
 *  Author: bun
 */ 

#include "extender.h"

#ifdef EXTENDER_USE_PORT

/*void initExtender(uint8_t* mask){
	EXTENDER_DDR = mask;
}*/
void initExtender(){
	extenderClear();
}

void setExtenderValue(uint8_t* val){
	EXTENDER_PORT = val;
}
void extenderClear(){
	uint8_t *extenderZeroBuffer = (uint8_t*)calloc(EXTENDER_BYTES, sizeof(uint8_t));
	setExtenderValue(extenderZeroBuffer);
	free(extenderZeroBuffer);
}
#endif



