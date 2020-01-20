/*
 * extender.h
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */

#ifndef EXTENDER_H_
#define EXTENDER_H_

#include "config.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(EXTENDER_USE_SN74HC164)
	#include "hardware/SN74HC164.h"
#elif defined(EXTENDER_USE_SN74HC595)
	#include "hardware/SN74HC595.h"
#elif defined(EXTENDER_USE_PORT)
	#define E_REGNAME2(type,suffix) type ## suffix
	#define E_REGNAME(type,suffix) E_REGNAME2(type,suffix)
	#define EXTENDER_PORT E_REGNAME(PORT, EXTENDER_USE_PORT)
	#define EXTENDER_DDR E_REGNAME(DDR, EXTENDER_USE_PORT)
	
	#if EXTENDER_BITS!=8 
		#error "EXTENDER_BITS must always be 8 when using EXTENDER_USE_PORT"
	#endif
#endif

void setExtenderValue(uint8_t* val);
void initExtender();
void extenderClear();

#ifdef __cplusplus
}
#endif

#endif /* EXTENDER_H_ */
