/*
 * PowerMgmt.h
 *
 * Created: 09.01.2020 13:46:43
 *  Author: bun
 */ 


#ifndef POWERMGMT_H_
#define POWERMGMT_H_

#include <stdbool.h>
#include "modular/Module.h"

typedef enum{
	PS_ON = 0,
	PS_OFF = 1,
	PS_DEEPSLEEP_REQUESTED = 3,
	PS_DEEPSLEEP = 1,
	PS_DEEPSLEEP_SHORTWAKE = 2,
}powerState;

#define EVENT_DEEP_SLEEP_REQUESTED 0x30
#define EVENT_DEEP_SLEEP_ENTER 0x31
#define EVENT_DEEP_SLEEP_WAKEUP 0x32
#define EVENT_DEEP_SLEEP_SHORTWAKE 0x33

#define PWRMGMT_DS_INTERVAL_MS 5000
#define PWRMGMT_DS_SHORTWAKE_MS 250

#define PWRMGMT_DS_TIMER_CYCLES_PER_SECOND F_CPU / 1024 / 256

class PowerManagement : public Module{

	protected:
	void deepSleep(); // "user power down" - disable all modules except control, low rate polling
	void enterSleep();
	void exitDeepSleep();
	uint8_t deepSleepFramesLeft;
	
	public:
	volatile powerState state;
	
	PowerManagement();
	
	void requestDeepSleep(); // give 1s for all modules (dimming, comm, etc), then enter
	void wakeup(bool immediately = false);
	void shortWakeStart();
	void shortWakeEnd();
	void tick();
	void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	
};

#endif /* POWERMGMT_H_ */