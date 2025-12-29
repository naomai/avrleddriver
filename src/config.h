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
#include "mcu.h"

#define DEVICE_SERIAL 99 // for radio identification
#define ENTITY_COUNT 7 //number of led lights



/*      LED hardware config         */

// color calibration

#define CALIBRATION_TABLE { \
	{.dr=0xFF, .dg=0xD4, .db=0x91, .dc1=0x36, .dc2=0x00, .add1={.b=0x9E, .g=0xBB, .r=0xFF}}, \
	{.dr=0x00, .dg=0x00,. db=0x00, .dc1=0xFF, .dc2=0xFF, .add1={.b=0xFF, .g=0xFF, .r=0x00}, .add2={.b=0x00, .g=0xFF, .r=0xFF}} \
}
	
// for each light, assign type, extender channels for R,G,B,WHITE,WARM

// 001cocina
/*#define LIGHTS_CONFIG { \
	{.type=LIGHT_RGB, .pins={3,4,8}, .calibrationIdx=0}, \
	{.type=LIGHT_RGB, .pins={2,5,9}, .calibrationIdx=0}, \
	{.type=LIGHT_RGB, .pins={1,6,10}, .calibrationIdx=0}, \
	{.type=LIGHT_RGB, .pins={0,7,11}, .calibrationIdx=0}, \
} // */

// 002dormi
// notes
// W B R G 12
// G R B 12
/*#define LIGHTS_CONFIG { \
	{.type=LIGHT_RGB, .pins={14,13,15}, .calibrationIdx=0}, \
	{.type=LIGHT_RGBW, .pins={2,3,1,0}, .calibrationIdx=0}, \
	{.type=LIGHT_RGBW, .pins={6,7,5,4}, .calibrationIdx=0}, \
	{.type=LIGHT_RGBW, .pins={10,11,9,8}, .calibrationIdx=0}, \

/*	{.type=SENSOR_HUMIDITY, .pins={0}}, \
	{.type=SENSOR_TEMPERATURE, .pins={1}}, \
	{.type=SENSOR_BATTERY, .pins={2}}, \*/

// 099proto
#define LIGHTS_CONFIG { \
	{.type=LIGHT_RGB, .pins={0,1,2,0,0}, .calibrationIdx=0}, \
	{.type=LIGHT_CCT, .pins={0,0,0,3,4}, .calibrationIdx=1}, \
}

/*       output multiplexer       *
 *       aka extender             */

// select which output multiplier method (extender) is used

// direct AVR output
//#define EXTENDER_USE_PORT B

// shift registers from 7400-series, like SN74HC164 or SN74HC595
#define EXTENDER_USE_SN7400

#define SN7400_IC 164 // last number from part name
#define SN7400_CONTROL_PORT		PORTC
#define SN7400_CONTROL_DDR		DDRC

// use only those needed for your SR (see hardware/SN7400_hardware.h)
#define SN7400_CLEAR_PIN	    PC5
#define SN7400_LATCH_PIN	    PC1
#define SN7400_CS_PIN			PC2
#define SN7400_OE_PIN			PC3

// byte width of output register 
#define EXTENDER_BYTES 2

/*          features            */

// turn on lights after connecting to power
#define AUTO_POWERUP

// rotary encoder as input method <control/EncoderModule.cpp>
//#define ENCODER_ENABLE

// color configurator with predefined presets <control/Menu.cpp>
//#define MENU_ENABLE

// remote control with radio module <control/Radio.cpp>
#define RADIO_ENABLE
//#define RADIO_OLD_PINOUT // 001cocina

// remote control using UART interface (ESP32)
//#define UART_ENABLE

/*       rotary encoder         */

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


/*            radio              */
#define RFM73_CHANNEL 80
#define RFM73_ADDRESS_BASE 0x777000

#define FPS 30

#define PWM_RESOLUTION (1<<10)

#define PWM_STRETCH (PWM_RESOLUTION/256)
#define PWM_SLOWDOWN 2
//#define PWM_LONGPULSE 30 // (old) during longer PWM events SPI semaphore is released

// undefine if you need to use ADC for other purposes
#define ADC_EXCLUSIVE 


/* debug features */

// disable some features for debugging convenience in atmel studio:
// enhanced RNG, startup delays, input
//#define DEBUG_SIMULATOR 

// enable logging handler (increases size significantly)
// see comments in <debug.c>
//#define DEBUG_LOGGING 

// disable animations
//#define DEBUG_NOANIM

// toggle PC2 with each updateFrame (check for freezes)
#define DEBUG_FRAMESTROBE

// change color to random each frame (PWM stability test)
//#define DEBUG_RAINBOWTEST 

#define EXTENDER_BITS (EXTENDER_BYTES*8)

/*#if EXTENDER_BITS==8
typedef uint8_t EXTENDER_VARTYPE;
#elif EXTENDER_BITS==16
typedef uint16_t EXTENDER_VARTYPE;
#elif EXTENDER_BITS==32
typedef uint32_t EXTENDER_VARTYPE;
#endif*/

//typedef uint8_t* EXTENDER_VARTYPE;

#endif /* CONFIG_H_ */