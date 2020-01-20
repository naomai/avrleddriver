/*
 * extender.c
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */

#include "../mcu.h"
#include <util/delay.h>
#include "../extender.h"


#ifdef EXTENDER_USE_SN74HC164

//EXTENDER_VARTYPE extender;
//EXTENDER_VARTYPE extenderMask;

static void SN74HC164_Clear();

void SN74HC164_Init(){

	SN74HC164_NCLR_DDR |= 1 << SN74HC164_NCLR_BIT;
	SN74HC164_NCLR_PORT |=  (1<<SN74HC164_NCLR_BIT);
	DDRB |= (1<<PB5) | (1<<PB3) | (1<<PB2);
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA);
	//spiConfigAdd(PWM_THREAD_ID, (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA));
	//spiConfigAdd(PWM_THREAD_ID, (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1<<CPOL));
	SPSR = (1<<SPI2X);
	//extenderMask=mask;
}


void SN74HC164_Set(uint8_t* val) {
	SN74HC164_Clear();
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
}
static void SN74HC164_Clear(){ 
	SN74HC164_NCLR_PORT &= ~(1<<SN74HC164_NCLR_BIT);
	asm volatile("nop");
	SN74HC164_NCLR_PORT |= 1<<SN74HC164_NCLR_BIT;
}

#endif