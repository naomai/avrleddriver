/*
 * extender.c
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */

#include "../mcu.h"
#include <util/delay.h>
#include "../extender.h"


#ifdef EXTENDER_USE_SN74HC595

//EXTENDER_VARTYPE extender;
//EXTENDER_VARTYPE extenderMask;

static void SN74HC595_Apply();

void SN74HC595_Init(){
	SN74HC595_RCLK_DDR |= 1 << SN74HC595_RCLK_BIT;
	SN74HC595_RCLK_PORT &= ~(1<<SN74HC595_RCLK_BIT);
	DDRB |= (1<<PB5) | (1<<PB3) | (1<<PB2);
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA);
	SPSR = (1<<SPI2X);
	extenderClear();
	//extenderMask=mask;
}


void SN74HC595_Set(uint8_t* val) {
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
	
	SN74HC595_Apply();
}



static void SN74HC595_Apply(){ 
	SN74HC595_RCLK_PORT |= 1<<SN74HC595_RCLK_BIT;
	//asm volatile("nop");
	SN74HC595_RCLK_PORT &= ~(1<<SN74HC595_RCLK_BIT);
}

#endif