#include <avr/io.h>
#include "../../config.h"
#include "../types.h"
#include <stdint.h>

#ifdef RADIO_OLD_PINOUT
	#define PIN_IRQ PC4
	#define PIN_CE PC2
	#define PIN_CSN PC3
	#define SIGNAL_DDR DDRC
	#define SIGNAL_PORT PORTC
	#define INPUT_DDR DDRC
	#define INPUT_PIN PINC
#else
	#define PIN_IRQ PC5
	#define PIN_CE PD3
	#define PIN_CSN PD2
	#define SIGNAL_DDR DDRD
	#define SIGNAL_PORT PORTD
	#define INPUT_DDR DDRC
	#define INPUT_PIN PINC
#endif

void RFM73_InitInterface() BOOTLOADER_SECTION;
void RFM73_SelectChip(bool state) BOOTLOADER_SECTION;
void RFM73_EnableChip(bool state) BOOTLOADER_SECTION;
uint8_t RFM73_SwapByte(uint8_t byte) BOOTLOADER_SECTION;
bool RFM73_InterruptState() BOOTLOADER_SECTION;

void RFM73_InitInterface(){
	SIGNAL_DDR |= (1<<PIN_CSN) | (1<<PIN_CE); // CSN | CE
	INPUT_DDR &= ~(1<<PIN_IRQ); // IRQ
	
	UBRR0 = 0; // DS: To ensure that the XCK line is initialized correctly according to the SPI mode settings, it is important that the UBRR is set to zero at the time the transmitter is enabled.
	DDRD |= (1<<PD4); // XCLK
	UCSR0C = (1<<UMSEL01) | (1<<UMSEL00);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UBRR0 = 2;

	RFM73_SelectChip(false);
}

void RFM73_SelectChip(bool state){
	if(state){
		SIGNAL_PORT &= ~(1<<PIN_CSN);
	}else{
		SIGNAL_PORT |= (1<<PIN_CSN);
	}
}
void RFM73_EnableChip(bool state){
	if(!state){
		SIGNAL_PORT &= ~(1<<PIN_CE);
	}else{
		SIGNAL_PORT |= (1<<PIN_CE);
	}
}

uint8_t RFM73_SwapByte(uint8_t byte){
	// exact code from Atmega328P DS
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)));
	/* Put data into buffer, sends the data */
	UDR0 = byte;
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

bool RFM73_InterruptState(){
	return !(INPUT_PIN & (1<<PIN_IRQ));
}
