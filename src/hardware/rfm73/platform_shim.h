#if defined(__arm__)
	#include <wiringPi.h>
	#define BOOTLOADER_SECTION
	#define delayMs(x) delay(x)
	#define delayUs(x) delayMicroseconds(x)
#elif defined(__AVR_ARCH__)
	#include <util/delay.h>
	#include <avr/boot.h>
	#define delayMs(x) _delay_ms(x)
	#define delayUs(x) _delay_us(x)
#endif
