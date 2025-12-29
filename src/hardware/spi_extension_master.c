/*
 * spi_extension_master.c
 *
 * Created: 14.02.2020 12:34:25
 *  Author: bun
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include "spi_extension.h"

void spiex_select(bool state);

void spiex_init(){
	if(!(UCSR0B & (1<<RXEN0))) {
		UBRR0 = 0; // DS: To ensure that the XCK line is initialized correctly according to the SPI mode settings, it is important that the UBRR is set to zero at the time the transmitter is enabled.
		DDRD |= (1<<PD4); // XCLK
		UCSR0C = (1<<UMSEL01) | (1<<UMSEL00);
		UCSR0B = (1<<RXEN0)|(1<<TXEN0);
		UBRR0 = 2;
	}
	PORTC |= (1<<PC4);
	DDRC |= (1<<PC4); // CS output
}

void spiex_select(bool state){
	if(state){
		PORTC &= ~(1<<PC4);
	}else{
		PORTC |= (1<<PC4);
	}	
}

void spiex_exchange(spiexPacket *packet){
	uint8_t *packetBytes = (uint8_t*)&packet;
	spiex_select(true);
	for(uint8_t i=0; i<sizeof(spiexPacket); i++){
		// exact code from Atmega328P DS
		/* Wait for empty transmit buffer */
		while (!(UCSR0A & (1<<UDRE0)));
		/* Put data into buffer, sends the data */
		UDR0 = packetBytes[i];
		/* Wait for data to be received */
		while (!(UCSR0A & (1<<RXC0)));
		/* Get and return received data from buffer */
		packetBytes[i] = UDR0;
	}
	spiex_select(false);
}

void spiex_sleep(){
	spiexPacket sleepPacket;
	sleepPacket.type = SPIEX_PACKET_SLEEPMODE;
	spiex_exchange(&sleepPacket);
	
	// put bus in sleep mode (extension wakes master by setting MISO low)
	UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));
	PORTD &= ~(1<<PD0);
	DDRD |= (1<<PD0);
	
	// interrupt on state change (PCINT16)
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT16);
}

void spiex_wakeup(){
	PCMSK2 &= ~(1<<PCINT16);
	spiex_init();
	_delay_ms(1);
}

bool spiex_busy(){
	return false;	
}

ISR (PCINT2_vect){
	if(!(PINC & PC0)){		
		spiex_wakeup();
	}
}

