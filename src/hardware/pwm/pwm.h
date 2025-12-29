/*
 * pwm.h
 *
 * Created: 14.12.2019 14:45:41
 *  Author: bun
 */ 


#ifndef PWM_H_
#define PWM_H_

#include "../../types.h"
#include "../LedHardware.h"

#ifdef __cplusplus
extern "C" {
#endif

void initPwm();
void pwmPrepareStart();
void pwmBufferLight(light_s *s);
void pwmPrepareFinish() /*__attribute__((optimize("Os")))*/;
bool pwmIsReady();

extern uint8_t PWMChannels;
#ifdef __cplusplus
}
#endif

#endif /* PWM_H_ */