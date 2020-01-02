/*
 * encoder.h
 *
 *  Created on: 9 paü 2019
 *      Author: bun
 */

#ifndef ENCODER_H_
#define ENCODER_H_
#include "../mcu.h"
#include "../config.h"
#include <stdbool.h>


/*
#define ENCODER_PORT  PORTD
#define ENCODER_PIN   PIND
#define ENCODER_DDR   DDRD
#define ENCODER_PIN0  PD6
#define ENCODER_PIN1  PD7
#define ENCODER_BUT   PD5

#define ENCODER_PIN0_MASK (1<<PD7)
#define ENCODER_PIN1_MASK (1<<PD6)
#define ENCODER_BUT_MASK  (1<<PD5)*/

typedef enum {
	ENC_NONE, ENC_CW, ENC_CCW, ENC_BUTDOWN, ENC_BUTUP
} encoderEvent;

void initEncoder();
encoderEvent checkEncoder();
bool checkEncoderButton();

#endif /* ENCODER_H_ */
