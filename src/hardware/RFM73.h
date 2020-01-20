

#ifndef RFM73_H_
#define RFM73_H_

#include <avr/boot.h>
 
#ifdef __cplusplus
extern "C" {
#endif

void RFM73_Init() __attribute__((noinline)) BOOTLOADER_SECTION; 
void RFM73_InitInterface() __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_InitChip() __attribute__((optimize("Os"))) __attribute__((noinline)) BOOTLOADER_SECTION;
bool RFM73_IsRadioPresent() __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_ListenStart() __attribute__((optimize("Os"))) __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_ListenStop() __attribute__((optimize("Os"))) __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_SetDeviceAddress(uint32_t addr) BOOTLOADER_SECTION;
void RFM73_PowerUp() __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_PowerDown() __attribute__((noinline)) BOOTLOADER_SECTION;
bool RFM73_Transmit(uint32_t addr, uint8_t *data, uint8_t length) BOOTLOADER_SECTION;
//void RFM73_WriteAckPayload(uint8_t *data, uint8_t length);
bool RFM73_IsDataAvailable() BOOTLOADER_SECTION;
//bool RFM73_IsAckPayloadSent();
uint8_t RFM73_ReadData(uint8_t *buffer) __attribute__((noinline)) BOOTLOADER_SECTION;
void RFM73_SetChannel(uint8_t channel) BOOTLOADER_SECTION;
uint8_t RFM73_FlushTX() BOOTLOADER_SECTION;
uint8_t RFM73_FlushRX() BOOTLOADER_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* RFM73_H_ */