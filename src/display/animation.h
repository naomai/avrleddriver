/*
 * animation.h
 *
 * Created: 13.11.2019 12:44:14
 *  Author: bun
 */ 


#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <stdbool.h>
#include "color.h"

#define EVENT_ANIM_START 0x20
#define EVENT_ANIM_FINISH 0x21

typedef enum{
	ANIM_NONE = 0x0,
	ANIM_INTROCHAIN = 0x1,
	ANIM_REMOTE = 0x2,
	ANIM_TEMPFX = 0x3,
	ANIM_LOOP_MASK = 0x40,
	ANIM_LOOP_ALTERNATING = 0x41
} animType;

typedef struct {
	uint8_t stripId;
	colorRaw from;
	colorRaw to;
	uint16_t speed;
	uint16_t progress;
	animType tag;
} animation;

animation* animCreate(uint8_t stripId, colorRaw from, colorRaw to, uint16_t speed, animType tag);
void animStart(animation *a);
void animCancel(animation *a);
void animAdvanceAll();
bool animIsActive(animation *a);
bool animIsLooping(animation *a);

#endif /* ANIMATION_H_ */