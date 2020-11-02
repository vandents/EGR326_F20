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
    SysTick->CTRL = 0x00000005;	// Enable SysTick, 3MHz, no interrupts
}

/**
 * SysTick delay milliseconds
 */
void SysTick_Delay_ms(uint16_t delay) {
    SysTick->LOAD = ((delay * 3000) - 1);	// Delay for 1ms per delay value
    SysTick->VAL = 0;						// Reset count
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}

/**
 * SysTick delay microseconds
 */
void SysTick_Delay_us(uint16_t delay) {
	SysTick->LOAD = ((delay * 3) - 1);		// Delay for 1us per delay value
	SysTick->VAL = 0;				 		// Any write to CVR clears it
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}
