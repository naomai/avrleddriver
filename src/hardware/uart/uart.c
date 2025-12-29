#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include "../../config.h"
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"

#define UART_CALC_DIVISOR(x) ((F_CPU + x * 8UL) / (16UL * x) - 1)

#if defined(__AVR_ATmega8__)

#define UART_DATA_REG        UDR
#define UART_STATUS_REG_A    UCSRA
#define UART_STATUS_REG_B    UCSRB
#define UART_STATUS_REG_C    UCSRC
#define UART_BAUD_REG_L      UBRRL
#define UART_BAUD_REG_H      UBRRH
#define UART_RX_VECTOR       USART_RXC_vect
#define UART_TX_VECTOR       USART_TXC_vect
#define UART_UDRE_VECTOR     USART_UDRE_vect

#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168P__) || \
      defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#define UART_DATA_REG        UDR0
#define UART_STATUS_REG_A    UCSR0A
#define UART_STATUS_REG_B    UCSR0B
#define UART_STATUS_REG_C    UCSR0C
#define UART_BAUD_REG_L      UBRR0L
#define UART_BAUD_REG_H      UBRR0H
#define UART_RX_VECTOR       USART_RX_vect
#define UART_TX_VECTOR       USART_TX_vect
#define UART_UDRE_VECTOR     USART_UDRE_vect

#define RXCIE RXCIE0
#define RXEN RXEN0
#define TXEN TXEN0
#define UDRIE UDRIE0
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01

#endif

void UART_StartTransmission();
void UART_EndTransmission();



// circular buffers 
uint8_t bufferTx[UART_TX_BUFFER_SIZE];
UART_BUFFER_SIZE_TYPE bufferTxHead = 0; // offset for next input byte
UART_BUFFER_SIZE_TYPE bufferTxTail = 0; // offset of next byte to transmit

uint8_t bufferRx[UART_RX_BUFFER_SIZE];
UART_BUFFER_SIZE_TYPE bufferRxHead = 0; // offset for next incoming byte
UART_BUFFER_SIZE_TYPE bufferRxTail = 0; // offset of next byte for read


void UART_InitInterface() {
	bufferTxHead = 0;
	bufferTxTail = 0;
	bufferRxHead = 0;
	bufferRxTail = 0;

	UART_BAUD_REG_H = (UART_CALC_DIVISOR(UART_BAUDRATE) >> 8);
	UART_BAUD_REG_L = (UART_CALC_DIVISOR(UART_BAUDRATE) & 0xFF);
	UART_STATUS_REG_B = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);

	// 8 bits, no parity, 1 stop bit
	#ifdef URSEL
		// ATmega8 weird register UCSRC/UBBRH
		UART_STATUS_REG_C = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
	#else
		UART_STATUS_REG_C = (1<<UCSZ1) | (1<<UCSZ0);
	#endif
}

UART_BUFFER_SIZE_TYPE UART_GetRxBytesCount() {
	return (bufferRxHead - bufferRxTail + UART_RX_BUFFER_SIZE) % UART_RX_BUFFER_SIZE;
}

UART_BUFFER_SIZE_TYPE UART_GetTxBytesCount() {
	return (bufferTxHead - bufferTxTail + UART_TX_BUFFER_SIZE) % UART_TX_BUFFER_SIZE;
}

ISR(UART_RX_VECTOR) {
	bufferRx[bufferRxHead] = UART_DATA_REG;
	bufferRxHead = (bufferRxHead + 1) % UART_RX_BUFFER_SIZE;
}

ISR(USART_UDRE_vect) {
	//if(UART_GetTxBytesCount()) {
	if(bufferTxHead != bufferTxTail) {
		UART_DATA_REG = bufferTx[bufferTxTail];
		bufferTxTail = (bufferTxTail + 1) % UART_TX_BUFFER_SIZE;
	} else {
		UART_EndTransmission();
	}
}

void UART_Write(uint8_t* bytes, UART_BUFFER_SIZE_TYPE count) {
	for(uint8_t i = 0; i<count; i++) {
		UART_WriteByte(bytes[i]);
	}
}

void UART_WriteByte(uint8_t byte) {
	bufferTx[bufferTxHead] = byte;
	bufferTxHead = (bufferTxHead + 1) % UART_TX_BUFFER_SIZE;

	UART_StartTransmission();
}

UART_BUFFER_SIZE_TYPE UART_Read(uint8_t *bytes, UART_BUFFER_SIZE_TYPE count) {
	UART_BUFFER_SIZE_TYPE countBuffer = UART_GetRxBytesCount();
	if (countBuffer < count) {
		count = countBuffer;
	}

	if(count==0) {
		return 0;
	}
	for(UART_BUFFER_SIZE_TYPE i = 0; i<count; i++) {
		bytes[i] = bufferRx[bufferRxTail];
		bufferRxTail = (bufferRxTail + 1) % UART_RX_BUFFER_SIZE;
	}
	return count;
}

UART_BUFFER_SIZE_TYPE UART_SkipRxBytes(UART_BUFFER_SIZE_TYPE count) {
	UART_BUFFER_SIZE_TYPE countBuffer = UART_GetRxBytesCount();
	if (countBuffer < count) {
		count = countBuffer;
	}

	if(count==0) {
		return 0;
	}
	bufferRxTail = (bufferRxTail + count) % UART_RX_BUFFER_SIZE;
	return count;
}

uint8_t UART_ReadByte() {
	UART_BUFFER_SIZE_TYPE countBuffer = UART_GetRxBytesCount();
	uint8_t byte; 

	if (!countBuffer) {
		return 0;
	}
	byte = bufferRx[bufferRxTail];
	bufferRxTail = (bufferRxTail + 1) % UART_RX_BUFFER_SIZE;
	return byte;
}

uint8_t UART_PeekByte() {
	return bufferRx[bufferRxTail];
}

void UART_StartTransmission() {
	// enable UDRE interrupt, which will feed UART TX controller with bytes from buffer
	UART_STATUS_REG_B |= (1 << UDRIE);
}

void UART_EndTransmission() {
	// disable UDRE interrupt to prevent it from constantly hammering the execution
	UART_STATUS_REG_B &= ~(1 << UDRIE);
}
