/*
 * Time.h
 *
 *  Created on: Oct 31, 2020
 *      Author: scottvandentoorn
 */

#ifndef TIME_H_
#define TIME_H_


#include "msp.h"


#include "msp.h"

/* Clock sources/frequencies */
void Clock_48MHz_Init(void);

/* SysTick */
void SysTick_Init(void);
void SysTick_Delay_ms(uint16_t delay);
void SysTick_Delay_us(uint16_t delay);

/* Timer32 */
void Timer32_1_Interrupt_Init(int hertz);

#endif /* TIME_H_ */
