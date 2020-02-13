#include "../../config.h"
#include <avr/io.h>
#include <stdbool.h>
#include <avr/boot.h>

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

void RFM73_InitInterface() __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_SelectChip(bool state) BOOTLOADER_SECTION;
void RFM73_EnableChip(bool state) BOOTLOADER_SECTION;
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

bool RFM73_InterruptState(){
	return !(INPUT_PIN & (1<<PIN_IRQ));
}
