

#include "mcu.h"
#include "types.h"
#include <avr/interrupt.h>
#include <util/delay.h> 
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdlib.h>
#include <stdbool.h>

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
#include "control/UARTController.h"
#include "control/SPIExtension.h"
#include "PowerMgmt.h"
#include "sensor/SensorHub.h"
#include "sensor/DHT.h"

void updateFrame();

void softReset();
void preventSoftReset() __attribute__((naked)) __attribute__((section(".init3")));

void* operator new(size_t size) { return calloc(1,size); }
void operator delete(void* ptr) { free(ptr); }

Dispatcher *dispatcher;
LedDriver *lights;
SensorHub *sensors;
DHT *DHTSensor;
Encoder *enc;
Menu *menu;
Settings *settings;
Radio *rf;
UARTController *uart;
SPIExtension *spiex;
PowerManagement *pwr;

bool requestFxFrame = false, schedulerAction = false;
uint16_t frameId = 0;
uint8_t timer0overflows = 0;


int main(){
	cli();
	PORTB=255;
	PORTC=255;
	PORTD=255;
	initDebugFeatures();
	
	initExtender();
	
	
	initRandom();
	
	dispatcher = new Dispatcher();
	
	settings = new Settings();
	dispatcher->registerModule(settings);
	lights = new LedDriver();
	dispatcher->registerModule(lights);	
	#ifdef SENSORS_ENABLE
		sensors = new SensorHub();
		dispatcher->registerModule(sensors);
		#ifdef SENSORS_DHT_ENABLE
			DHTSensor = new DHT();
			dispatcher->registerModule(DHTSensor);
		#endif
	#endif
	#ifdef ENCODER_ENABLE
		enc = new Encoder();
		dispatcher->registerModule(enc);
	#endif
	#ifdef MENU_ENABLE
		menu = new Menu();
		dispatcher->registerModule(menu);
	#endif

	#ifdef RADIO_ENABLE
		rf = new Radio();
		dispatcher->registerModule(rf);
		rf->signIn(DEVICE_SERIAL);
	#endif

	#ifdef UART_ENABLE
		uart = new UARTController();
		dispatcher->registerModule(uart);
	#endif

	
	#ifdef SPIEX_ENABLE
		spiex = new SPIExtension();
		dispatcher->registerModule(spiex);
	#endif
	
	// must be registered last
	pwr = new PowerManagement();
	dispatcher->registerModule(pwr);
	
	dispatcher->lateInitAll();
	
	#ifdef AUTO_POWERUP
	{
		dispatcher->event_queue->pushEvent(EVENT_POWERSTATE, 255, PS_ON, 0);
	}
	#else
		pwr->requestDeepSleep();
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
		if(requestFxFrame){
			updateFrame();
			requestFxFrame=false;
			randomFeedEntropy();
		}
		dispatcher->tick();
	}
}

ISR(TIMER0_OVF_vect){
	if (/*(pwr->state == PS_ON || pwr->state == PS_DEEPSLEEP_REQUESTED)
		&&*/ ++timer0overflows >= F_CPU / FPS / 256 / 64
	   ) {
		timer0overflows = 0;
		requestFxFrame = 1;
	}
}



void updateFrame(){
	dispatcher->event_queue->pushEvent(EVENT_FRAME, frameId >> 8, frameId & 0xFF);
	animAdvanceAll();
	
	frameId++;
	
	#ifdef DEBUG_FRAMESTROBE
		PORTC ^= (1<<PC2);
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