/*
 * extender.c
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */

#include "../mcu.h"
#include <util/delay.h>
#include "../extender.h"
#include "SN7400_hardware.h"


#ifdef EXTENDER_USE_SN7400


void SN7400_Init(){

	SETUP_CLR();
	SETUP_LATCH();
	SETUP_CS();
	SETUP_OE();
	
	DDRB |= (1<<PB5) | (1<<PB3) | (1<<PB2);
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA);
	#if defined(SN7400_CLOCK_PULSE_LOW)
		SPCR |= (1<<CPOL);
	#endif
	#if !defined(SN7400_HALFSPEED)
		SPSR = (1<<SPI2X);
	#endif
	
	SR_CLR();
}


void SN7400_Set(uint8_t* val) {
	SR_SELECT();
	SR_OUTPUT_DISABLE();
	SN7400_Clear();
	#if EXTENDER_BYTES==1
		SPDR = val & 0xFF;
		while(! bit_is_set( SPSR, SPIF ) );
	#else
		uint8_t *ptr = &val[EXTENDER_BYTES-1];
		while(1){
			SPDR = *ptr;
			if(ptr == val)
				break;
			ptr--;
			while( ! bit_is_set( SPSR, SPIF ) );
		}
	#endif
	while( ! bit_is_set( SPSR, SPIF ) );
	
	SR_LATCH();
	SR_OUTPUT_ENABLE();
	SR_UNSELECT();
}

void SN7400_Clear(){
	SR_CLR();
}


#endif