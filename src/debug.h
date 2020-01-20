/*
 * debug.h
 *
 * Created: 12.11.2019 22:42:22
 *  Author: bun
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

void initDebugFeatures();

#if defined DEBUG_LOGGING
	//#include "uart.h"
	void _log_src(char *msg, char *file, int line) __attribute__((optimize("O0")));
	void _logf_src(char *msg, char *file, int line,...) __attribute__((optimize("O0")));
	
	#define _log(x) _log_src((char *)x,(char *)__FILE__,__LINE__)
	#define _logf(x,...) _logf_src((char *)x,(char *)__FILE__,__LINE__,__VA_ARGS__)
	
	#define dumpRam(s,e) for(uint8_t *i=(uint8_t*)(((uint16_t)s)&0xFFF0); i<(uint8_t*)(e); i++){\
			if((uint16_t)i % 16 == 0){\
				_logf("\n%04x | ", i);\
			}\
			_logf(" %02x", *i);\
		}
#else
	#define _log(x)
	#define _logf(x,...)
	#define dumpRam(s,e)
#endif

#ifdef __cplusplus
};
#endif

#endif /* DEBUG_H_ */