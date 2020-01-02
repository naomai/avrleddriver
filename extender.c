/*
 * CFile1.c
 *
 * Created: 12.11.2019 21:09:38
 *  Author: bun
 */ 
#include "extender.h"

#ifdef EXTENDER_USE_PORT

void initExtender(EXTENDER_VARTYPE mask){
	EXTENDER_DDR = mask;
}

void setExtenderValue(EXTENDER_VARTYPE val){
	EXTENDER_PORT = val;
}

#endif

