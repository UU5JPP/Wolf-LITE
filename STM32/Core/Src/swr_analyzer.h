#ifndef SWR_ANALYZER_H
#define SWR_ANALYZER_H

#include "stm32f4xx.h"
#include "main.h"
#include "stdbool.h"

#define SWR_StepDelay 1     //scan delay, msec
#define SWR_VParts 6        //vertical signatures
#define SWR_TopSWR 10.0f

//Public variabled
extern bool sysmenu_swr_opened;

//Public methods
extern void SWR_Start(uint32_t start, uint32_t end);                 //analyzer launch
extern void SWR_Stop(void);									//stop session
extern void SWR_Draw(void);                  //drawing analyzer
extern void SWR_EncRotate(int8_t direction); //analyzer events per encoder

#endif
