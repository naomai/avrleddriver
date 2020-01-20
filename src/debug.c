/*
 * debug.c
 *
 * Created: 13.11.2019 13:36:11
 *  Author: bun
 */ 

#include "debug.h"
#include <stdio.h>
#include <stdarg.h>

// debug features init
// if using AtmelStudio, you can route the log messages to output window with breakpoints
// place breakpoints in marked places, tick Action and paste in output message: "{output,s}"

void initDebugFeatures(){
	#ifdef DEBUG_FRAMESTROBE
		DDRC |= (1<<PC2);
	#endif
}

// uart logging

void _log_src(char *msg, char *file, int line){
	char output[140];
	sprintf(output, "[%s:%i] %s",file, line, msg);
	asm volatile("nop\n"); // << place breakpoint here
}

void _logf_src(char *msg, char *file, int line, ...){
	char output[140];
	va_list args;
	va_start(args, line);
	vsprintf(output, msg, args);
	va_end(args);
	_log_src(output, file, line);
}