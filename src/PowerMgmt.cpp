/*
 * PowerMgmt.cpp
 *
 * Created: 09.01.2020 13:46:18
 *  Author: bun
 */ 

#include "PowerMgmt.h"
#include "config.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "control/EncoderModule.h"

extern PowerManagement *pwr;
volatile uint8_t dsTimerOverflows = 0;

PowerManagement::PowerManagement(){
	deepSleepFramesLeft = 255; 
	state = PS_ON;
	set_sleep_mode(SLEEP_MODE_IDLE);
	
	/*
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<5);
	DDRC |= (1<<PC3);//dbg
	*/
}

// make sure the PowerManagement module is initialized last
void PowerManagement::tick(){
	enterSleep();
}

void PowerManagement::requestDeepSleep(){
	this->raiseEvent(EVENT_DEEP_SLEEP_REQUESTED, 0, 0, true);
}

void PowerManagement::wakeup(bool immediately){
	if(pwr->state != PS_ON){
		this->raiseEvent(EVENT_DEEP_SLEEP_WAKEUP, 0, 0, !immediately);
		if(immediately){
			exitDeepSleep();
		}
	}
}

void PowerManagement::shortWakeStart(){
	state = PS_DEEPSLEEP_SHORTWAKE;
	this->raiseEvent(EVENT_DEEP_SLEEP_SHORTWAKE, 0, 0, false);
	set_sleep_mode(SLEEP_MODE_IDLE);
	//PORTC |= (1<<PC3);
}

void PowerManagement::shortWakeEnd(){
	this->raiseEvent(EVENT_DEEP_SLEEP_ENTER, 0, 0, true);
	set_sleep_mode(SLEEP_MODE_IDLE);
	//PORTC &= ~(1<<PC3);
}


void PowerManagement::event(uint8_t type, uint8_t lbyte, uint8_t hbyte){
	/*if(type==EVENT_INPUT_KEY && lbyte == KEYSTATE_UP && hbyte == KEY_ENTER){
		requestDeepSleep();
	}*/
	if(type==EVENT_FRAME && state == PS_DEEPSLEEP_REQUESTED){
		if(deepSleepFramesLeft-- == 0){
			this->raiseEvent(EVENT_DEEP_SLEEP_ENTER, 0, 0, true);
			this->raiseEvent(EVENT_POWERSTATE,255,PS_OFF);
		}
	}else if(type==EVENT_DEEP_SLEEP_REQUESTED){
		deepSleepFramesLeft = FPS;
		state = PS_DEEPSLEEP_REQUESTED;
	}else if(type==EVENT_DEEP_SLEEP_ENTER &&
		(state == PS_DEEPSLEEP_REQUESTED || state==PS_DEEPSLEEP_SHORTWAKE)){
			deepSleep();
	}else if(type==EVENT_DEEP_SLEEP_WAKEUP){
		exitDeepSleep();
	}
}

void PowerManagement::deepSleep(){
	state = PS_DEEPSLEEP;
	
	TCCR2B |= (1<<CS22) |  (1<<CS21) |  (1<<CS20);
	TIMSK2 |= (1<<TOIE2);
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	//PRR = 0xFF;
	do{
		enterSleep();
	}while(state==PS_DEEPSLEEP);
}

void PowerManagement::exitDeepSleep(){
	if(pwr->state != PS_ON){
		TIMSK2 &= ~(1<<TOIE2);
		state = PS_ON;
		set_sleep_mode(SLEEP_MODE_IDLE);
		
		this->raiseEvent(EVENT_POWERSTATE,255,PS_ON);
	}
}

void PowerManagement::enterSleep(){
	cli();
	sleep_enable();
	sleep_bod_disable();
	sei();
	sleep_cpu();
	sleep_disable();
}

/*
ISR (PCINT2_vect){
	pwr->wakeup(true);
}*/

ISR (TIMER2_OVF_vect){
	if(pwr->state == PS_DEEPSLEEP){
		if(++dsTimerOverflows >= PWRMGMT_DS_TIMER_CYCLES_PER_SECOND * PWRMGMT_DS_INTERVAL_MS / 1000){
			pwr->shortWakeStart();
			dsTimerOverflows = 0;
		}
	}else if(pwr->state == PS_DEEPSLEEP_SHORTWAKE){
		if(++dsTimerOverflows >= PWRMGMT_DS_TIMER_CYCLES_PER_SECOND * PWRMGMT_DS_SHORTWAKE_MS / 1000){
			pwr->shortWakeEnd();
			dsTimerOverflows = 0;
		}
	}	
}