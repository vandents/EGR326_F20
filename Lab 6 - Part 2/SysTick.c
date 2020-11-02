/*
 * SysTick.c
 *
 *  Created on: Oct 1, 2020
 *      Author: scottvandentoorn
 */

#include "SysTick.h"


/**
 * SysTick Timer initialization
 */
void SysTick_Init(void) {
    SysTick->CTRL = 0;			// Disable SysTick
    SysTick->LOAD = 0x00FFFFFF;	// Max reload value
    SysTick->VAL = 0;			// Reset count
    SysTick->CTRL = 0x00000005;	// Enable SysTick, core clock, no interrupts
}

/**
 * SysTick delay milliseconds. Recursive if delay > 349 ms
 */
void SysTick_Delay_ms(uint16_t delay) {
	// Delay for 1ms per delay value
	if (delay > 349) SysTick->LOAD = 349 * 48000 - 1;
	else SysTick->LOAD = delay * 48000 - 1;

    SysTick->VAL = 0;						// Reset count
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET

    if (delay > 349) SysTick_Delay_ms(delay - 349);
}

/**
 * SysTick delay microseconds
 */
void SysTick_Delay_us(uint16_t delay) {
	SysTick->LOAD = delay * 12 - 1;		// Delay for 1us per delay value
	SysTick->VAL = 0;				 		// Any write to CVR clears it
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}
