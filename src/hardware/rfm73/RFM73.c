/*
 * RFM73.c
 *
 * Created: 17.12.2019 10:52:35
 *  Author: bun
 */ 

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "RFM73.h"
#include "defs.h"
#include <avr/pgmspace.h>
#include "platform_shim.h"

#define count(x) sizeof(x)/sizeof(x[0])




#define PACKET_LEN 32


typedef enum{
	TRX_TRANSMITTER,
	TRX_RECEIVER
} trxMode;


void RFM73_TransmitStart(uint32_t addr) BOOTLOADER_SECTION;
void RFM73_SelectChip(bool state);
void RFM73_EnableChip(bool state);
bool RFM73_InterruptState();
uint8_t RFM73_SwapByte(uint8_t byte);
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

struct {
	uint16_t totalTX;
	uint16_t totalRX;
	uint16_t totalPL;
	uint16_t totalRT;
} radioStats;

uint32_t myAddress = 0;

const uint8_t PROGMEM REGS1[][4] ={
	//MIXED BYTE ORDER!! Check docs in SPI Command section
	/* MS-byte to LS-byte in Bank1 registers 0-8 & 15 */
	/* LS-byte to MS-byte in Bank0 & Bank1 registers 9-14 */
	{0x40, 0x4B, 0x01, 0xE2}, // Must write with 0x404B01E2
	{0xC0, 0x4B, 0x00, 0x00}, // Must write with 0xC04B0000
	{0xD0, 0xFC, 0x8C, 0x02}, // Must write with 0xD0FC8C02
		
		
	// more info: AN0007(BK2423)
	// REG3
	//{0x99, 0x00, 0x39, 0x41}, // Must write with 0x99003941
	{0xF9, 0x00, 0x39, 0x41}, // BK2423 - high transmission power mode
		//bit31~29: 111; Rvco_tx[2:0]=Bank1_REG3[31:29]
	
	// REG4
	//{0xD9, 0xBE, 0x86, 0x0B}, // BK2423 - Must write with 0xD99E860B (sic!)
		 //bit21: High sensitivity mode (different CD detection values))
	{0xF9, 0xBE, 0x84, 0x0B}, // BK2423 - high transmission power mode 
		 //bit20:      1; RF_PWR[2]=Bank1_REG4[20]
		 //bit09:      0; IctrlPA[0]=Bank1_REG4[9]
		 //bit29~27: 111; txIctrl[2:0]=Bank1_REG4[29:27]
	//{0xF9, 0x96, 0x82, 0x1B}, //  BK2425 - 1Msps: 0xF996821B
		
	// REG5
	{0x24, 0x02, 0x0F, 0xA6}, // Must write with 0x24060FA6 (sic! bit18: RSSI measurement Enable)
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x12, 0x73, 0x05}, // Please initialize with 0x05731200
	{0x36, 0xb4, 0x80, 0x00}  // Please initialize with 0x0080B436
};

const uint8_t PROGMEM REG1_E[] ={
	// BK2423: Ramp curve, MS-to-LS 0xFFEF7DF208082082041041
	0xFF, 0xEF, 0x7D, 0xF2, 0x08, 0x08, 0x20, 0x82, 0x04, 0x10, 0x41
	// BK2425: Ramp curve, MS-to-LS 0xFFFFFEF7CF208104082041
	//0xFF, 0xFF, 0xFE, 0xF7, 0xCF, 0x20, 0x81, 0x04, 0x08, 0x20, 0x41
};

void RFM73_Init(){
	RFM73_InitInterface();
	if(RFM73_IsRadioPresent()){
		RFM73_InitChip();
	}
}

void RFM73_InitChip(){
	RFM73_SwitchBank(0);

	RFM73_PowerDown();
	
	RFM73_UglyInit();
	
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
	
	RFM73_WriteRegisterByte(SETUP_RETR, 0x26); // 500us, 6 retransmissions
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
	RFM73_FlushTX();
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
	RFM73_PowerDown();
	RFM73_FlushTX();
	RFM73_FlushRX();
	RFM73_SetTRXMode(TRX_TRANSMITTER);
	RFM73_WriteAddressRegs(addr);
	RFM73_PowerUp();
}



void RFM73_PowerUp(){

	RFM73_RegisterOr(EN_RXADDR, (1<<ERX_P0));
	uint8_t power = RFM73_ReadRegisterByte(NRF_CONFIG) & (1<<PWR_UP);
	if(!power){
		RFM73_RegisterOr(NRF_CONFIG, (1<<PWR_UP));
		delayMs(2);
	}
}

void RFM73_PowerDown(){
	RFM73_EnableChip(false);
	RFM73_RegisterAnd(EN_RXADDR, ~(1<<ERX_P0));
	RFM73_RegisterAnd(NRF_CONFIG, ~(1<<PWR_UP));
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
	RFM73_WriteRegister(RF_CH, &chanMasked, 1);
}

void RFM73_WriteAddressRegs(uint32_t addr){
	RFM73_WriteRegister(RX_ADDR_P0, &addr, 4);
	RFM73_WriteRegister(TX_ADDR, &addr, 4);
}

void RFM73_SetTRXMode(trxMode mode){
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

	for(uint8_t i=0; i<length; i++){
		RFM73_SwapByte(data[i]);
	}
	RFM73_SelectChip(false);
	delayUs(5);
	RFM73_EnableChip(true);
	delayUs(15);
	RFM73_EnableChip(false);
	
	uint8_t wait=0;
	do{
		delayUs(100);
		wait++;
	}while(!RFM73_InterruptState() && wait < 53);
	
	uint8_t status = RFM73_GetStatus();
	bool success = false;

	if(status & (1<<TX_DS)){
		RFM73_WriteRegisterByte(NRF_STATUS,(1<<TX_DS));
		success = true;	
	}else if(status & (1<<MAX_RT)){
		RFM73_FlushTX();
		RFM73_WriteRegisterByte(NRF_STATUS,(1<<MAX_RT));
	}else{
		RFM73_PowerDown();
		RFM73_ListenStop();
	}

	RFM73_WriteRegisterByte(NRF_STATUS, 0x70);
	
	RFM73_ListenStart();
	return success;
}


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
	return length;
}


void RFM73_EnableNewFeatures(){
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
	// check radio presence by switching register banks
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
		uint8_t pipe5addr;
		RFM73_ReadRegister(0xF, &pipe5addr, 1);
		radioPresent = (pipe5addr==0xC6);
	}
	
	return radioPresent;
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

uint8_t RFM73_FlushTX(){
	uint8_t status;
	RFM73_SelectChip(true);
	status=RFM73_SwapByte(FLUSH_TX);
	RFM73_SelectChip(false);
	RFM73_WriteRegisterByte(NRF_STATUS,(1<<TX_DS)|(1<<MAX_RT));
	return status;
}

uint8_t RFM73_FlushRX(){
	uint8_t status;
	RFM73_SelectChip(true);
	status=RFM73_SwapByte(FLUSH_RX);
	RFM73_SelectChip(false);
	RFM73_WriteRegisterByte(NRF_STATUS,(1<<RX_DR));
	return status;
}

bool RFM73_IsDataAvailable(){
	if(RFM73_InterruptState()){
		uint8_t fifo = RFM73_ReadRegisterByte(FIFO_STATUS);
		if(!(fifo & (1<<RX_EMPTY))){
			RFM73_RegisterOr(NRF_STATUS, (1<<RX_DR));
			return true;
		}
	}
	return false;
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
	
	/*//http://www.voti.nl/rfm73/index.html
	memcpy_P(&regVal, &REG1_4, sizeof(REG1_4));
	RFM73_WriteRegister(0x04, &regVal, 4);*/
	
	memcpy_P(&regValE, &REG1_E, sizeof(REG1_E));
	RFM73_WriteRegister(0x0E, &regValE, sizeof(REG1_E));
	
	
	
	RFM73_SwitchBank(0);
}