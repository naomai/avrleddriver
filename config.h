/*
 * config.h
 *
 * Created: 08.11.2019 17:06:49
 *  Author: bun
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
//#include "types.h"
#include "mcu.h"
//#include "display/ledstrips.h"

#define DEVICE_SERIAL 001 // for radio identification
#define STRIPS_COUNT 4 //number of led strips

// for each strip, assign extender channel for R,G,B
#define RGB_CALIB {.dr=0xFF, .dg=0xFC, .db=0xEB}
#define STRIPS_CONFIG { \
	{.type=LIGHT_RGB, .pins={3,4,8}, .calibration=RGB_CALIB}, \
	{.type=LIGHT_RGB, .pins={2,5,9}, .calibration=RGB_CALIB}, \
	{.type=LIGHT_RGB, .pins={1,6,10}, .calibration=RGB_CALIB}, \
	{.type=LIGHT_RGB, .pins={0,7,11}, .calibration=RGB_CALIB} \
} // (sic) My PCB has G&B reversed*/

/*#define CCT_CALIB {.dr=0xFF,.dg=0xFF,.db=0xFF,.add1={.b=0xFF, .g=0x00, .r=0x00}, .add2={.b=0x00, .g=0x00, .r=0xFF}}

#define STRIPS_CONFIG { \
	{.type=LIGHT_CCT, .pins={0,0,0,1,2}, .calibration=CCT_CALIB} \
}// */

#define EXTENDER_BITS 16

// output multiplier config
// select which output multiplier method (extender) is used

// direct AVR output
//#define EXTENDER_USE_PORT B

// 8-bit shift register, SPI with additional ~CLR signal
#define EXTENDER_USE_SN74HC164 

//SN74HC164 extender config
//used only for ~CLR signal
#define SN74HC164_CLR_PORT   PORTC
#define SN74HC164_CLR_DDR    DDRC
#define SN74HC164_CLR_BIT    PC5

//rotary encoder
#define ENCODER_PORT  PORTD
#define ENCODER_PIN   PIND
#define ENCODER_DDR   DDRD
#define ENCODER_PIN0  PD7
#define ENCODER_PIN1  PD6
#define ENCODER_BUT   PD5

#define ENCODER_PIN0_MASK (1<<PD7)
#define ENCODER_PIN1_MASK (1<<PD6)
#define ENCODER_BUT_MASK  (1<<PD5)

#define EEPROM_CONFIG_LOC 0x00

#define RADIO_ENABLE // enable radio module support (SPI, aimed for RFM73)
#define RFM73_USE_PORT C
#define RFM73_PIN_CE 2
#define RFM73_PIN_CSN 3
#define RFM73_PIN_IRQ 4
#define RFM73_CHANNEL 80
#define RFM73_ADDRESS_BASE 0x777000

//#define I2C_ENABLE_REMOTE // enable I2C slave mode, redirect input to readSettingsFromStream()

// SPI sharing
//#define SPI_MULTIPLE_DEVICES // enable when both SPI extender and radio module are used
#define SPI_DEVICES_COUNT 2
#define PWM_THREAD_ID 0
#define RADIO_THREAD_ID	1

#define FPS 30

#define PWM_RESOLUTION (1<<10)

#define PWM_STRETCH (PWM_RESOLUTION/256)
#define PWM_SLOWDOWN 2
#define PWM_SHORTPULSE 10 // short PWM events are exclusively occupying CPU time (accuracy)
#define PWM_LONGPULSE 30 // (SPI_MULTIPLE_DEVICES) during longer PWM events SPI semaphore is released

/* debug features */

// enable logging handler (increases size significantly)
// see comments in <debug.c>
//#define DEBUG_LOGGING 

// disable animations
//#define DEBUG_NOANIM

// disable some features for debugging convenience in atmel studio:
// enhanced RNG, startup delays, input
//#define DEBUG_SIMULATOR 

// toggle PC1 with each updateFrame (check for freezes)
//#define DEBUG_FRAMESTROBE

// change color to random each frame (PWM stability test)
//#define DEBUG_RAINBOWTEST 



#if EXTENDER_BITS==8
typedef uint8_t EXTENDER_VARTYPE;
#elif EXTENDER_BITS==16
typedef uint16_t EXTENDER_VARTYPE;
#elif EXTENDER_BITS==32
typedef uint32_t EXTENDER_VARTYPE;
#endif

#endif /* CONFIG_H_ */