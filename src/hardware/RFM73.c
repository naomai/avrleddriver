/*
 * RFM73.c
 *
 * Created: 17.12.2019 10:52:35
 *  Author: bun
 */ 

#include "../config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "RFM73.h"
#include "../types.h"
#include "../debug.h"

#define PACKET_LEN 32

#ifdef RADIO_OLD_PINOUT
	#define PIN_IRQ PC4
	#define PIN_CE PC2
	#define PIN_CSN PC3
	#define SIGNAL_DDR DDRC
	#define SIGNAL_PORT PORTC
	#define INPUT_DDR DDRC
	#define INPUT_PIN PINC
#else
	#define PIN_IRQ PC5
	#define PIN_CE PD3
	#define PIN_CSN PD2
	#define SIGNAL_DDR DDRD
	#define SIGNAL_PORT PORTD
	#define INPUT_DDR DDRC
	#define INPUT_PIN PINC
#endif

const uint8_t PROGMEM REGS1[][4] ={
	{0x40, 0x4B, 0x01, 0xE2},
	{0xC0, 0x4B, 0x00, 0x00},
	{0xD0, 0xFC, 0x8C, 0x02},
	{0x99, 0x00, 0x39, 0x41},
	{0xDF, 0x9E, 0x86, 0x0B},
	{0x24, 0x06, 0x7F, 0xA6},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x12, 0x73, 0x05},
	{0x36, 0xB4, 0x80, 0x00}
};

const uint8_t PROGMEM REG1_4[] ={
	0xD9, 0x9E, 0x86, 0x0B
};

const uint8_t PROGMEM REG1_E[] ={
	0x41, 0x10, 0x04, 0x82, 0x20, 0x08, 0x08, 0xF2, 0x7D, 0xEF, 0xFF
	//0x41, 0x20, 0x08, 0x04, 0x81, 0x20, 0xCF, 0xF7, 0xFE, 0xFF, 0xFF
};

typedef enum{
	TRX_TRANSMITTER,
	TRX_RECEIVER
} trxMode;

//bool waitingACKPLD = false;

void RFM73_TransmitStart(uint32_t addr) BOOTLOADER_SECTION;
void RFM73_SelectChip(bool state) BOOTLOADER_SECTION;
void RFM73_EnableChip(bool state) BOOTLOADER_SECTION;
uint8_t RFM73_SwapByte(uint8_t byte) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_SwitchBank(uint8_t bank) BOOTLOADER_SECTION;
void RFM73_Reset();
void RFM73_SetTRXMode(trxMode mode);
void RFM73_EnableNewFeatures();
uint8_t RFM73_ReadRegisterByte(uint8_t reg) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_ReadRegister(uint8_t reg, void* data, size_t length);
uint8_t RFM73_WriteRegisterByte(uint8_t reg, uint8_t data) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_WriteRegister(uint8_t reg, void* data, size_t length) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_RegisterAnd(uint8_t addr, uint8_t mask) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_RegisterOr(uint8_t addr, uint8_t mask) __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_WriteAddressRegs(uint32_t addr) __attribute__((noinline)) BOOTLOADER_SECTION;
uint8_t RFM73_GetStatus();
void RFM73_UglyInit();

/*typedef enum{
	RFMSTATE_OFF = 0,
	RFMSTATE_LISTENING = 1,
	RFMSTATE_TRANSMITTING = 2,
	RFMSTATE_STANDBY = 3
}rmfState;

rmfState radioState;*/

struct {
	uint16_t totalTX;
	uint16_t totalRX;
	uint16_t totalPL;
	uint16_t totalRT;
} radioStats;

uint32_t myAddress = 0;

void RFM73_Init(){
	RFM73_InitInterface();
	if(RFM73_IsRadioPresent()){
		RFM73_InitChip();
	}
}

void RFM73_InitInterface(){
	SIGNAL_DDR |= (1<<PIN_CSN) | (1<<PIN_CE); // CSN | CE
	INPUT_DDR &= ~(1<<PIN_IRQ); // IRQ
	
	UBRR0 = 0; // DS: To ensure that the XCK line is initialized correctly according to the SPI mode settings, it is important that the UBRR is set to zero at the time the transmitter is enabled.
	DDRD |= (1<<PD4); // XCLK
	UCSR0C = (1<<UMSEL01) | (1<<UMSEL00);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UBRR0 = 2;

	RFM73_SelectChip(false);
}

void RFM73_InitChip(){
	RFM73_SwitchBank(0);

	RFM73_PowerDown();
	
	//RFM73_UglyInit();
	
	// setup address width to 4 bytes
	RFM73_WriteRegisterByte(SETUP_AW, 0x02);
	//250kbps, +5 dBm, LNA high gain
	//RFM73_WriteRegisterByte(RF_SETUP, (1<<RF_DR_LOW)|(1<<RF_PWR_HIGH)|(1<<RF_PWR_LOW)|(1<<LNA_HCURR));
	//1mbps, +5 dBm, LNA high gain
	RFM73_WriteRegisterByte(RF_SETUP,(1<<RF_PWR_HIGH)|(1<<RF_PWR_LOW)|(1<<LNA_HCURR));
	
	RFM73_WriteRegisterByte(NRF_CONFIG,(1<<CRCO)|(1<<EN_CRC));
	//auto ack
	RFM73_WriteRegisterByte(EN_AA, (1<<ENAA_P0));
	//retransmission
	
	RFM73_WriteRegisterByte(SETUP_RETR, 0x34); // 750ms, 4 retransmissions
	//from datasheet of nRF24L01+:
	//"In 250kbps mode (even when the payload is not in ACK) the ARD must be 500uS or more."
	
	//dynamic packet length
	RFM73_EnableNewFeatures();
	RFM73_WriteRegisterByte(FEATURE, /*(1<<EN_ACK_PAY) |*/ (1<<EN_DPL) | (1<<EN_DYN_ACK));
	RFM73_WriteRegisterByte(DYNPD, 0x3F);
	
	//RFM73_WriteRegisterByte(RX_PW_P0, PACKET_LEN);
	
	// undocumented init of status registers
	RFM73_WriteRegisterByte(NRF_STATUS, 0x70);
	RFM73_WriteRegisterByte(OBSERVE_TX, 0x00);
	RFM73_WriteRegisterByte(FIFO_STATUS, 0x00);
	
	RFM73_FlushTX();
	RFM73_FlushRX();

}



void RFM73_ListenStart(){
	//radioState = RFMSTATE_LISTENING;
	//RFM73_EnableChip(false);
	//RFM73_PowerDown();
	RFM73_FlushTX();
	//RFM73_FlushRX();
	RFM73_WriteAddressRegs(myAddress);
	RFM73_SetTRXMode(TRX_RECEIVER);
	RFM73_PowerUp();
		
	RFM73_EnableChip(true);
}

void RFM73_ListenStop(){
	RFM73_PowerDown();
	RFM73_SetTRXMode(TRX_TRANSMITTER);
}

void RFM73_TransmitStart(uint32_t addr){
	//radioState = RFMSTATE_STANDBY;
	RFM73_PowerDown();
	RFM73_FlushTX();
	RFM73_FlushRX();
	RFM73_SetTRXMode(TRX_TRANSMITTER);
	RFM73_WriteAddressRegs(addr);
	RFM73_PowerUp();
	
	//RFM73_EnableChip(false);
	//RFM73_PowerDown();
}



void RFM73_PowerUp(){

	RFM73_RegisterOr(EN_RXADDR, (1<<ERX_P0));
	uint8_t power = RFM73_ReadRegisterByte(NRF_CONFIG) & (1<<PWR_UP);
	if(!power){
		RFM73_RegisterOr(NRF_CONFIG, (1<<PWR_UP));
		_delay_ms(2);
	}
	
	//radioState = RFMSTATE_STANDBY;
}

void RFM73_PowerDown(){
	RFM73_EnableChip(false);
	RFM73_RegisterAnd(EN_RXADDR, ~(1<<ERX_P0));
	RFM73_RegisterAnd(NRF_CONFIG, ~(1<<PWR_UP));
	//radioState = RFMSTATE_OFF;
}

void RFM73_Reset(){
	RFM73_InitChip();
	RFM73_PowerUp();
}

void RFM73_SetDeviceAddress(uint32_t addr){
	RFM73_WriteAddressRegs(addr);
	myAddress = addr;
}

void RFM73_SetChannel(uint8_t channel){
	uint8_t chanMasked = channel & 0x7F;
	//RFM73_SwitchBank(0);
	RFM73_WriteRegister(RF_CH, &chanMasked, 1);
}

void RFM73_WriteAddressRegs(uint32_t addr){
	//RFM73_SwitchBank(0);
	RFM73_WriteRegister(RX_ADDR_P0, &addr, 4);
	RFM73_WriteRegister(TX_ADDR, &addr, 4);
}

void RFM73_SetTRXMode(trxMode mode){
	//RFM73_SwitchBank(0);
	uint8_t conf = RFM73_ReadRegisterByte(NRF_CONFIG);
	if (mode == TRX_RECEIVER) {
		conf |= (1<<PRIM_RX);
	}else{
		conf &= ~(1<<PRIM_RX);
	}
	RFM73_WriteRegisterByte(NRF_CONFIG, conf);
}

bool RFM73_Transmit(uint32_t addr, uint8_t *data, uint8_t length){
	RFM73_TransmitStart(addr);
		
	RFM73_SelectChip(true);
	RFM73_SwapByte(W_TX_PAYLOAD);
	/*for(uint8_t i=0; i<PACKET_LEN; i++){
		RFM73_SwapByte(i<length ? data[i] : 0x00);
	}*/
	for(uint8_t i=0; i<length; i++){
		RFM73_SwapByte(data[i]);
	}
	RFM73_SelectChip(false);
	
	RFM73_EnableChip(true);
	_delay_us(15);
	RFM73_EnableChip(false);
	
	uint8_t wait=0;
	do{
		_delay_us(100);
		wait++;
	}while((INPUT_PIN & (1<<PIN_IRQ)) && wait < 53);
	
	//radioStats.totalTX++;
	
	uint8_t status = RFM73_GetStatus();
	bool success = false;

	if(status & (1<<TX_DS)){
		RFM73_WriteRegisterByte(NRF_STATUS,(1<<TX_DS));
		success = true;	
		_log("TX OK");
	}else if(status & (1<<MAX_RT)){
		RFM73_FlushTX();
		RFM73_WriteRegisterByte(NRF_STATUS,(1<<MAX_RT));
		//radioStats.totalPL++;
		_log("Timeout");
	}else{
		//chip timeout todo
		RFM73_PowerDown();
		RFM73_ListenStop();
		_log("RadioFail");
		//radioStats.totalPL++;
		
	}
	
	/*if(RFM73_IsDataAvailable()){
		//PORTB |= (1<<PB1);
		//RFM73_FlushRX();
		_log("ACK PLD");
	}else{
		//PORTB &= ~(1<<PB1);
	}*/
	
	
	//PORTB ^= (1<<PB1);
	RFM73_WriteRegisterByte(NRF_STATUS, 0x70);
	//RFM73_FlushTX();
	
	RFM73_ListenStart();
	return success;
}

/*void RFM73_WriteAckPayload(uint8_t *data, uint8_t length){
	RFM73_FlushTX();
	RFM73_SelectChip(true);
	RFM73_SwapByte(W_ACK_PAYLOAD);

	for(uint8_t i=0; i<length; i++){
		RFM73_SwapByte(data[i]);
	}
		
	RFM73_SelectChip(false);
	
	waitingACKPLD = true;
}*/

uint8_t RFM73_ReadData(uint8_t *buffer){
	RFM73_SelectChip(true);
	RFM73_SwapByte(R_RX_PL_WID);
	uint8_t length = RFM73_SwapByte(RF24_NOP);
	RFM73_SelectChip(false);
	
	
	RFM73_SelectChip(true);
	RFM73_SwapByte(R_RX_PAYLOAD);
	for(uint8_t i=0; i<length; i++){
		buffer[i] = RFM73_SwapByte(0xFF);
	}	
	RFM73_SelectChip(false);
	//_delay_us(50);
	//RFM73_FlushTX();
	//RFM73_FlushRX();
	return length;
}

void RFM73_SelectChip(bool state){
	if(state){
		SIGNAL_PORT &= ~(1<<PIN_CSN);
	}else{
		SIGNAL_PORT |= (1<<PIN_CSN);
	}
}
void RFM73_EnableChip(bool state){
	if(!state){
		SIGNAL_PORT &= ~(1<<PIN_CE);
	}else{
		SIGNAL_PORT |= (1<<PIN_CE);
	}
}

void RFM73_EnableNewFeatures(){
	//RFM73_SwitchBank(0);
	RFM73_WriteRegisterByte(FEATURE, (1<<EN_DPL));
	uint8_t feat = RFM73_ReadRegisterByte(FEATURE);
	if(!feat) {
		RFM73_SelectChip(true);
		RFM73_SwapByte(ACTIVATE);
		RFM73_SwapByte(0x73);
		RFM73_SelectChip(false);
	}
	RFM73_WriteRegisterByte(FEATURE, 0);
}

bool RFM73_IsRadioPresent(){
	uint8_t status, bankStart;
	bool radioPresent = false;

	RFM73_SelectChip(true);
	status = RFM73_SwapByte(ACTIVATE);
	bankStart = (status >> 7);
	RFM73_SwapByte(0x53);
	RFM73_SelectChip(false);
	
	status = RFM73_GetStatus();
	radioPresent = (status >> 7) != bankStart;
	
	if(radioPresent){
		RFM73_SwitchBank(0);

	}
	
	return radioPresent;
}

uint8_t RFM73_SwapByte(uint8_t byte){
	// exact code from Atmega328P DS
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)));
	/* Put data into buffer, sends the data */
	UDR0 = byte;
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
	
	/*SPDR = byte;
	while(! bit_is_set(SPSR, SPIF));
	return SPDR;*/
}

uint8_t RFM73_SwitchBank(uint8_t bank){
	uint8_t status = RFM73_GetStatus();
	uint8_t currentBank = (status >> 7);
	if (currentBank != bank){
		RFM73_SelectChip(true);
		status = RFM73_SwapByte(ACTIVATE);
		RFM73_SwapByte(0x53);
		RFM73_SelectChip(false);
	}
	return status;
}

uint8_t RFM73_ReadRegisterByte(uint8_t reg){
	RFM73_SelectChip(true);
	RFM73_SwapByte(reg & 0x1F);
	uint8_t data=RFM73_SwapByte(RF24_NOP);
	RFM73_SelectChip(false);
	return data;
}
uint8_t RFM73_ReadRegister(uint8_t reg, void *data, size_t length){
	uint8_t status;
	RFM73_SelectChip(true);
	status = RFM73_SwapByte(reg & 0x1F);
	uint8_t *charPtr = (uint8_t*) data;
	for(uint8_t i=0; i<length; i++){
		charPtr[i]=RFM73_SwapByte(RF24_NOP);
	}	
	RFM73_SelectChip(false);
	return status;
}

uint8_t RFM73_WriteRegisterByte(uint8_t reg, uint8_t data){
	RFM73_SelectChip(true);
	uint8_t status = RFM73_SwapByte((reg & 0x1F) | W_REGISTER);
	RFM73_SwapByte(data);
	RFM73_SelectChip(false);
	return status;
}

uint8_t RFM73_WriteRegister(uint8_t reg, void * data, size_t length){
	uint8_t status;
	RFM73_SelectChip(true);
	status = RFM73_SwapByte((reg & 0x1F) | W_REGISTER);
	uint8_t *charPtr = (uint8_t*) data;
	for(uint8_t i=0; i<length; i++){
		RFM73_SwapByte(charPtr[i]);
	}
	RFM73_SelectChip(false);
	return status;
}


uint8_t RFM73_RegisterAnd(uint8_t addr, uint8_t mask){
	uint8_t value = RFM73_ReadRegisterByte(addr);
	RFM73_WriteRegisterByte(addr, value & mask);
	return value;
}

uint8_t RFM73_RegisterOr(uint8_t addr, uint8_t mask){
	uint8_t value = RFM73_ReadRegisterByte(addr);
	RFM73_WriteRegisterByte(addr, value | mask);
	return value;
}

uint8_t RFM73_GetStatus(){
	RFM73_SelectChip(true);
	uint8_t status = RFM73_SwapByte(0xFF);
	RFM73_SelectChip(false);
	return status;
}

void RFM73_UglyInit(){
	// initialize registers with "mystery magic values"
	uint8_t regVal[4], regValE[sizeof(REG1_E)];
		
	/*// init PLL_LOCK (from 'RFM73 Replace RFM70 Precautions')
	RFM73_WriteRegisterByte(RF_SETUP, 0x0F);*/
	
	RFM73_SwitchBank(1);
	for(uint8_t i=0; i<count(REGS1); i++){
		memcpy_P(&regVal, &REGS1[i], sizeof(REGS1[0]));
		RFM73_WriteRegister(i, &regVal, 4);
	}
	
	//http://www.voti.nl/rfm73/index.html
	memcpy_P(&regVal, &REG1_4, sizeof(REG1_4));
	RFM73_WriteRegister(0x04, &regVal, 4);
	
	memcpy_P(&regValE, &REG1_E, sizeof(REG1_E));
	RFM73_WriteRegister(0x0E, &regValE, sizeof(REG1_E));
	
	
	
	RFM73_SwitchBank(0);
}

uint8_t RFM73_FlushTX(){
	uint8_t status;
	//RFM73_SwitchBank(0);
	//RFM73_RegisterAnd(NRF_CONFIG, 0xFE);
	RFM73_SelectChip(true);
	status=RFM73_SwapByte(FLUSH_TX);
	RFM73_SelectChip(false);
	RFM73_WriteRegisterByte(NRF_STATUS,(1<<TX_DS)|(1<<MAX_RT));
	//waitingACKPLD = false;
	return status;
}

uint8_t RFM73_FlushRX(){
	uint8_t status;
	//RFM73_SwitchBank(0);
	RFM73_SelectChip(true);
	status=RFM73_SwapByte(FLUSH_RX);
	RFM73_SelectChip(false);
	RFM73_WriteRegisterByte(NRF_STATUS,(1<<RX_DR));
	return status;
}

bool RFM73_IsDataAvailable(){
	if(!(INPUT_PIN & (1<<PIN_IRQ))){
		uint8_t fifo = RFM73_ReadRegisterByte(FIFO_STATUS);
		if(!(fifo & (1<<RX_EMPTY))){
			RFM73_RegisterOr(NRF_STATUS, (1<<RX_DR));
			return true;
		}
	}
	return false;
}


/*
bool RFM73_IsAckPayloadSent(){
	bool result=false;
	if(waitingACKPLD){
		RFM73_RegisterOr(NRF_CONFIG, (1<<MASK_RX_DR));
		uint8_t us50=0;
		do{
			_delay_us(50);
			us50++;
		}while((RFM73_PIN & (1<<RFM73_PIN_IRQ)) && us50 < 10);
		RFM73_RegisterAnd(NRF_CONFIG, ~(1<<MASK_RX_DR));
		waitingACKPLD = false;
		result = RFM73_ReadRegisterByte(NRF_STATUS) & (1<<TX_DS);
		RFM73_RegisterOr(NRF_STATUS, (1<<TX_DS));
	}
	return result;
}*/