/*
 * Time.h
 *
 *  Created on: Oct 31, 2020
 *      Author: scottvandentoorn
 */

#ifndef TIME_H_
#define TIME_H_


#include "msp.h"


/* Clock sources/frequencies */
void Clock_Frequencies_Init(void);

/* SysTick */
void SysTick_Init(void);
void SysTick_Delay_ms(uint16_t delay);
void SysTick_Delay_us(uint16_t delay);

/* Timer32 */
void Timer32_1_Interrupt_Init(int hertz);
void Timer32_2_Counter_Init(void);

/* Timer A */
void TimerA0_0_Interrupt_s_Init(uint16_t seconds);
void TimerA1_0_Interrupt_ms_Init(uint16_t millis);


#endif /* TIME_H_ */
