/*
 * spishare.h
 *
 * Created: 18.12.2019 17:53:57
 *  Author: bun
 */ 


#ifndef SPISHARE_H_
#define SPISHARE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "config.h"

void spiConfigAdd(uint8_t threadId, uint8_t regSPCR);
void spiBigThreadLock();
void spiBigThreadUnlock();
void spiWait();
bool spiCheckLock();
void spiSmallThreadLock();
void spiSmallThreadUnlock();


#define SPI_GUARD(x) if(!spiCheckLock()){spiSmallThreadLock(); spiConfigSwitch(x)
#define SPI_GUARD_END() spiSmallThreadUnlock();}

#define SPI_WAIT(x) spiWait(); spiSmallThreadLock(); spiConfigSwitch(x)
#define SPI_WAIT_END() spiSmallThreadUnlock()

extern uint8_t SPIConfigs[SPI_DEVICES_COUNT];

/*#ifdef SPI_MULTIPLE_DEVICES
#define spiConfigSwitch(x) SPCR=SPIConfigs[x]
#else*/
#define spiConfigSwitch(x)
//#endif

#ifdef __cplusplus
}
#endif

#endif /* SPISHARE_H_ */