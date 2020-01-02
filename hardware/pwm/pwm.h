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

void initPwm();
void pwmPrepareStart();
void pwmBufferLight(light_s *s);
void pwmPrepareFinish() /*__attribute__((optimize("Os")))*/;
bool pwmIsReady();

#endif /* PWM_H_ */