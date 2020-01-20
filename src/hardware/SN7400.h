/*
 * IncFile1.h
 *
 * Created: 12.11.2019 21:13:26
 *  Author: bun
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#define initExtender SN7400_Init
#define setExtenderValue SN7400_Set
#define extenderClear SN7400_Clear


void SN7400_Init();
void SN7400_Clear();
void SN7400_Set(uint8_t* val);

#endif /* INCFILE1_H_ */