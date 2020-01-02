

#include "mcu.h"
#include "types.h"
#include <avr/interrupt.h>
#include <util/delay.h> 
#include <avr/wdt.h>
#include <stdlib.h>

#include "extender.h"
#include "display/color.h"
#include "LedDriver.h"
#include "random.h"
#include <string.h>
#include "debug.h"

#include "modular/Dispatcher.h"
#include "control/EncoderModule.h"
#include "control/Menu.h"
#include "SettingsModule.h"
#include "control/Radio.h"

void updateFrame();

void softReset();
void preventSoftReset() __attribute__((naked)) __attribute__((section(".init3")));

void* operator new(size_t size) { return calloc(1,size); }
void operator delete(void* ptr) { free(ptr); }

Dispatcher *dispatcher;
LedDriver *lights;
Encoder *enc;
Menu *menu;
Settings *settings;
Radio *rf;

uint8_t requestFxFrame = 0;
uint16_t frameId = 0;
uint8_t timer0overflows = 0;


int main(){
	cli();
	PORTB=255;
	PORTC=255;
	PORTD=255;
	initDebugFeatures();
	
	initRandom();
	
	dispatcher = new Dispatcher();
	
	settings = new Settings();
	dispatcher->registerModule(settings);
	lights = new LedDriver();
	dispatcher->registerModule(lights);
	enc = new Encoder();
	dispatcher->registerModule(enc);
	menu = new Menu();
	dispatcher->registerModule(menu);

	#ifdef RADIO_ENABLE
	rf = new Radio();
	dispatcher->registerModule(rf);
	rf->signIn(DEVICE_SERIAL);
	#endif
	
	dispatcher->lateInitAll();
	
	initExtender(0xFFFF);
	setExtenderValue(0x00);

	#if !defined DEBUG_SIMULATOR
	if(enc->checkEncoderButton()){
		setExtenderValue(0xAA);
		_delay_ms(500);
		setExtenderValue(0x55);
		_delay_ms(500);
	}
	#endif

	DDRB |= 1;

	//Frame Timer
	TCNT0 = 0;
	#if defined TIMSK // mega8
		TCCR0 |= (1 << CS01) | (1 << CS00);
		TIMSK |= (1<<OCIE1A) | (1<<TOIE0);
	#elif defined TIMSK0 //mega328
		TCCR0B |= (1 << CS01) | (1 << CS00);
		TIMSK0 |= (1<<TOIE0);
		TIMSK1 |= (1<<OCIE1A);
	#endif
	sei();
	


	while(1){
		
		dispatcher->tick();
		if(requestFxFrame){
			updateFrame();
			requestFxFrame=0;
			randomFeedEntropy();
		}
	}
}

ISR(TIMER0_OVF_vect){
	if (++timer0overflows == F_CPU / FPS / 256 / 64) {
		timer0overflows = 0;
		requestFxFrame = 1;
	}
}



void updateFrame(){
	eventDescriptor newFrameEvent;
	newFrameEvent.type = EVENT_FRAME;
	newFrameEvent.hbyte = frameId >> 8;
	newFrameEvent.lbyte = frameId & 0xFF;
	dispatcher->queue->pushEvent(newFrameEvent);

	animAdvanceAll();
	
	frameId++;
	
	#ifdef DEBUG_FRAMESTROBE
		PORTC ^= (1<<PC1);
	#endif
}



void softReset(){
	wdt_enable(WDTO_15MS);  
	while(1){}
}

void preventSoftReset(){
	#if defined MCUSR
	MCUSR = 0;
	#else
	MCUCSR = 0;
	#endif
	wdt_disable();
	return;
}