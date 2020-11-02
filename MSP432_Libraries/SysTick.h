/*
 * SysTick.h
 *
 *  Created on: Oct 1, 2020
 *      Author: scottvandentoorn
 */

#ifndef INCLUDE_SYSTICK_H_
#define INCLUDE_SYSTICK_H_

#include "msp.h"


void SysTick_Init(void);
void SysTick_Delay_ms(uint16_t delay);
void SysTick_Delay_us(uint16_t delay);


#endif /* INCLUDE_SYSTICK_H_ */
