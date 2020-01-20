/*
 * mytypes.h
 *
 * Created: 08.11.2019 17:04:44
 *  Author: bun
 */ 


#ifndef MYTYPES_H_
#define MYTYPES_H_

#include <stdbool.h>
#include "config.h"
//#include "display/color.h"
//#include "display/ledstrips.h"

typedef enum {
	SET_H,
	SET_S,
	SET_V,
	SET_ID,
	SET_PRESET,
	SET_NOTHING,
	SET_FADEIN
} setterMode;

typedef enum {
	P_SPECIAL_RANDOM = 0x01,
	P_SPECIAL_CYCLING = 0x02,
	P_SPECIAL_ANIMATED = 0x80,
	P_SPECIAL_DONTRESET = 0x40,
	P_SPECIAL_POWERDOWN = 0x10
} specialPresets;


typedef uint16_t PWMOFFSET_VARTYPE;

//extern setterMode mode;

// https://stackoverflow.com/posts/3553321/revisions
#define member_size(type, member) sizeof(((type *)0)->member)

#define count(x) sizeof(x)/sizeof(x[0])

#endif /* MYTYPES_H_ */
