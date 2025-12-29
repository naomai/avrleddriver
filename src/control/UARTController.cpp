#include <string.h>
#include "../config.h"
#include "UARTController.h"
#include "../hardware/uart/uart.h"

#include "../LedDriver.h"

extern LedDriver *lights;


UARTController::UARTController() : Module() {
    UART_InitInterface();
    
}


void UARTController::tick(){
    uartFrameHeader frameHeader;
    uint8_t bytesAvailable = UART_GetRxBytesCount();
    uint8_t frameLength = 0;

	if(!bytesAvailable) {
        return;
    }
    frameLength = UART_PeekByte();
    if(frameLength ==0 || frameLength > bytesAvailable - 1) {
        return;
    }
    
    UART_Read((uint8_t*)&frameHeader, sizeof(frameHeader));
    switch(frameHeader.command) {
        case ucmd_Echo:
            processEchoCommand(frameHeader);
            break;
        case ucmd_SetImmediateState:
            processStateCommand(frameHeader);
            break;
        default:
            // unknown command, skip to the end of frame
            UART_SkipRxBytes(frameLength-1);
            break;
    }
    
}

void UARTController::processEchoCommand(uartFrameHeader requestHeader) {
    uartFrameHeader responseHeader;
    uint16_t echoBytes;
    if(requestHeader.length != 3) {
        // invalid length
        return;
    }
    UART_Read((uint8_t*)&echoBytes, sizeof(echoBytes));
    responseHeader.length = 3;
    responseHeader.command = ursp_Echo;
    UART_Write((uint8_t*)&responseHeader, sizeof(responseHeader));
    UART_Write((uint8_t*)&echoBytes, sizeof(echoBytes));
}

void UARTController::processStateCommand(uartFrameHeader requestHeader) {
    uartFrameHeader responseHeader;
    uartChannelStateFrame channelState;
    LedLight* light;
    colorRaw colorState;

    uint8_t expectedBytes = 0;
    lightColorType stateType;

    switch(requestHeader.command) {
        case ucmd_SetImmediateState:
            expectedBytes = sizeof(channelState);
            stateType = LIGHT_COLOR_DISPLAY;
            responseHeader.length = 0;
            break;
        case ucmd_GetImmediateState:
            expectedBytes = 2;
            stateType = LIGHT_COLOR_DISPLAY;
            responseHeader.length = 1 + sizeof(channelState);
            responseHeader.command = ursp_ImmediateState;
            break;
        case ucmd_SetPersistentState:
            expectedBytes = sizeof(channelState);
            stateType = LIGHT_COLOR_SET;
            responseHeader.length = 0;
            break;
        case ucmd_GetPersistentState:
            expectedBytes = 2;
            stateType = LIGHT_COLOR_SET;
            responseHeader.length = 1 + sizeof(channelState);
            responseHeader.command = ursp_ImmediateState;
            break;
        default: 
            return;
    }
    if(requestHeader.length - 1 != expectedBytes) {
        // invalid length
        return;
    }    
    UART_Read((uint8_t*)&channelState, expectedBytes);

    light = lights->getLightById(channelState.light);
    if(light == NULL) {
        // invalid light id
        return;
    }

    colorState = light->getColor(stateType, COLORSPACE_RAW);

    if(responseHeader.length > 0) { // setter
        ((uint8_t*)&colorState)[channelState.channel] = channelState.state >> 8;
		light->setColor(colorState, stateType, COLORSPACE_RAW);
    } else { // getter
        channelState.state = (((uint8_t*)&colorState)[channelState.channel] << 8);
        UART_Write((uint8_t*)&responseHeader, sizeof(responseHeader));
        UART_Write((uint8_t*)&channelState, sizeof(channelState));
    }


}

