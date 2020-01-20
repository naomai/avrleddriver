/*
 * SN7400_hardware.h
 *
 * Created: 20.01.2020 14:10:11
 *  Author: bun
 */ 


#ifndef SN7400_HARDWARE_H_
#define SN7400_HARDWARE_H_

#define PIN_SET_LOW(pin) \
	SN7400_CONTROL_PORT &= ~(1<<pin)

#define PIN_SET_HIGH(pin) \
	SN7400_CONTROL_PORT |= (1<<pin)

#define PULSE_LOW(pin) \
	PIN_SET_LOW(pin); \
	PIN_SET_HIGH(pin);

#define PULSE_HIGH(pin) \
	PIN_SET_HIGH(pin); \
	PIN_SET_LOW(pin);

#define PIN_CONFIG_LOW(pin) \
	PIN_SET_HIGH(pin); \
	SN7400_CONTROL_PORT |= (1<<pin);

#define PIN_CONFIG_HIGH(pin) \
	PIN_SET_HIGH(pin); \
	SN7400_CONTROL_PORT &= ~(1<<pin);


// shift registers definitions
// LOW = active low

// one stage - use "tricks" to minimize light leaks

#if SN7400_IC == 164
	// MISO: (A, B)
	// SCK: (CLK)
	// CLR: (~CLEAR) 
	#define SN7400_CLOCK_PULSE_LOW
	#define SN7400_CLEAR_LOW 
#elif SN7400_IC == 198 || SN7400_IC == 199
	// CLR: (~CLR) 
	// CS: (CLOCK INHIBIT) 
	#define SN7400_CLOCK_PULSE_LOW
	#define SN7400_CLEAR_LOW
	#define SN7400_CS_LOW 
#elif SN7400_IC == 322
	// CS: (~G)
	// OE: (OUTPUT ENABLE)
	#define SN7400_CLOCK_PULSE_LOW
	#define SN7400_CS_LOW
	#define SN7400_OE_LOW
	
// two stages (latch)

// keyword: 595
#elif (SN7400_IC >= 594 && SN7400_IC <= 599) || SN7400_IC == 675 || SN7400_IC == 4094 
	// MISO: (SER)
	// SCK: (~SRCLK/~SHCP/~CP)
	// LATCH: (RCLK/STCP/STROBE)
	#define SN7400_CLOCK_PULSE_LOW 
	#define SN7400_LATCH_HIGH
#endif

// signal polarity macros

#if defined(SN7400_CLEAR_LOW)
	#define SETUP_CLR() PIN_CONFIG_LOW(SN7400_CLEAR_PIN)
	#define SR_CLR() PULSE_LOW(SN7400_CLEAR_PIN)
#elif defined(SN7400_CLEAR_HIGH)
	#define SETUP_CLR() PIN_CONFIG_HIGH(SN7400_CLEAR_PIN)
	#define SR_CLR() PULSE_HIGH(SN7400_CLEAR_PIN)
#else
	#define SETUP_CLR()
	#define SR_CLR()
#endif

#if defined(SN7400_LATCH_LOW)
	#define SETUP_LATCH() PIN_CONFIG_LOW(SN7400_LATCH_PIN)
	#define SR_LATCH() PULSE_LOW(SN7400_LATCH_PIN)
#elif defined(SN7400_LATCH_HIGH)
	#define SETUP_LATCH() PIN_CONFIG_HIGH(SN7400_LATCH_PIN)
	#define SR_LATCH() PULSE_HIGH(SN7400_LATCH_PIN)
#else
	#define SETUP_LATCH()
	#define SR_LATCH()
#endif

#if defined(SN7400_CS_LOW)
	#define SETUP_CS() PIN_CONFIG_LOW(SN7400_CS_PIN)
	#define SR_SELECT() PIN_SET_LOW(SN7400_CS_PIN)
	#define SR_UNSELECT() PIN_SET_HIGH(SN7400_CS_PIN)
#elif defined(SN7400_CS_HIGH)
	#define SETUP_CS() PIN_CONFIG_HIGH(SN7400_CS_PIN)
	#define SR_SELECT() PIN_SET_HIGH(SN7400_CS_PIN)
	#define SR_UNSELECT() PIN_SET_LOW(SN7400_CS_PIN)
#else
	#define SETUP_CS()
	#define SR_SELECT()
	#define SR_UNSELECT()
#endif

#if defined(SN7400_OE_LOW)
	#define SETUP_OE() PIN_CONFIG_LOW(SN7400_OE_PIN)
	#define SR_OUTPUT_ENABLE() PIN_SET_LOW(SN7400_OE_PIN)
	#define SR_OUTPUT_DISABLE() PIN_SET_HIGH(SN7400_OE_PIN)
#elif defined(SN7400_OE_HIGH)
	#define SETUP_OE() PIN_CONFIG_HIGH(SN7400_OE_PIN)
	#define SR_OUTPUT_ENABLE() PIN_SET_HIGH(SN7400_OE_PIN)
	#define SR_OUTPUT_DISABLE() PIN_SET_LOW(SN7400_OE_PIN)
#else
	#define SETUP_OE()
	#define SR_OUTPUT_ENABLE()
	#define SR_OUTPUT_DISABLE()
#endif



#endif /* SN7400_HARDWARE_H_ */