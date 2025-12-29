

#ifndef UART_H_
#define UART_H_

#include <avr/boot.h>
 
#ifdef __cplusplus
extern "C" {
#endif

#define UART_BUFFER_SIZE_TYPE uint16_t
#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 256
#define UART_BAUDRATE 115200

UART_BUFFER_SIZE_TYPE UART_GetRxBytesCount();
UART_BUFFER_SIZE_TYPE UART_GetTxBytesCount();
UART_BUFFER_SIZE_TYPE UART_Read(uint8_t *bytes, UART_BUFFER_SIZE_TYPE count);
uint8_t UART_PeekByte();
UART_BUFFER_SIZE_TYPE UART_SkipRxBytes(UART_BUFFER_SIZE_TYPE count);
void UART_Write(uint8_t* bytes, UART_BUFFER_SIZE_TYPE count);
void UART_WriteByte(uint8_t byte);
void UART_InitInterface();


#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */