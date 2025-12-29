

#ifndef UARTCONTROLLER_H_
#define UARTCONTROLLER_H_


#include <stdbool.h>
#include "../modular/Module.h"

// FRAME STRUCTURE:
// (u8 length excluding this byte) (u8 command) (data)

#define UART_STATE_TYPE uint16_t

// commands received from ESP module
typedef enum{
	ucmd_Echo = 0,		
		// (u16 num) return ursp_Echo with u16 num unchanged 
	ucmd_SetImmediateState = 1, 
		// (u8 light, u8 channel, UART_STATE_TYPE state) 
		// change immediate (LIGHT_COLOR_DISPLAY) state of light channel (0 = off, 0xffff = fully on)
	ucmd_GetImmediateState = 2,
		// (u8 light, u8 channel) 
		// return ursp_ImmediateState
	ucmd_SetPersistentState = 3, 
		// (u8 light, u8 channel, UART_STATE_TYPE state) 
		// change persistent (LIGHT_COLOR_SET) state of light channel (0 = off, 0xffff = fully on)
	ucmd_GetPersistentState = 4,
		// (u8 light, u8 channel) 
		// return ursp_PersistentState

	ursp_Echo = 128,
		// u16 num from ucmd_Echo
	ursp_ImmediateState = 130,
		// UART_STATE_TYPE immediate (LIGHT_COLOR_DISPLAY) state of light channel (0 = off, 0xffff = fully on)
	ursp_PersistentState = 132
		// UART_STATE_TYPE persistent (LIGHT_COLOR_SET) state of light channel (0 = off, 0xffff = fully on)

} uartCommand;

typedef struct  {
	uint8_t length;
	uartCommand command;
} uartFrameHeader;

typedef struct {
	uint8_t light;
	uint8_t channel;
	UART_STATE_TYPE state;
} uartChannelStateFrame;

class UARTController : public Module{
	public:
	UARTController();
	~UARTController();
	//void event(uint8_t type, uint8_t lbyte, uint8_t hbyte);
	void tick();
	
	protected:
	void processEchoCommand(uartFrameHeader requestHeader);
	void processStateCommand(uartFrameHeader requestHeader);

};


#endif /* UARTCONTROLLER_H_ */
