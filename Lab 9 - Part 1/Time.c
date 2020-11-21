/*
 * Time.c
 *
 *  Created on: Oct 31, 2020
 *      Author: scottvandentoorn
 */


#include "Time.h"


/********************************************
 * Clock sources/frequencies
 ********************************************/

/**
 * Initialization for clock sources and frequencies.
 * Lecture slides and Valvano library used as reference.
 *
 * Clock frequencies and sources:
 * 	HFXTCLK = 48 MHz crystal
 * 	SMCLK = HFXTCLK / 4
 * 	HSMCLK = HFXTCLK / 2
 * 	MCLK = HFXTCLK
 * 	ACLK = REFOCLK
 */
void Clock_48MHz_Init(void) {
	// Configure Flash wait-state to 1 for banks 0 & 1
	FLCTL->BANK0_RDCTL =
		(FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK0_RDCTL_WAIT_1;
	FLCTL->BANK1_RDCTL =
		(FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK1_RDCTL_WAIT_1;

	// Configure HFXT to use 48MHz crystal
	PJ->SEL0 |= 0x0C;		// Configure PJ.2/3 for HFXTCLK function
	PJ->SEL1 &= ~0x0C;
	CS->KEY = CS_KEY_VAL;	// Unlock CS module
	CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;

	// Wait for the HFXT clock to stabilize
	while(CS->IFG & CS_IFG_HFXTIFG) CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

	// Set clock sources and dividers
	CS->CTL1 = 0x20000000 |	// SMCLK = HFXTCLK / 4
		0x00100000 |		// SMCLK = HFXTCLK / 2
		0x00000200 |		// ACLK source = REFOCLK
		0x00000055;			// SMCLK, HSMCLK, MCLK source = HFXTCLK

	CS->KEY = 0;
}


/**********************************************************
 * SysTick
 **********************************************************/

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
 * @param delay Delay in ms
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
	SysTick->LOAD = delay * 12 - 1;			// Delay for 1us per delay value
	SysTick->VAL = 0;				 		// Any write to CVR clears it
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}


/**********************************************************
 * Timer32
 **********************************************************/

/**
 * Configure T32.1 for interrupt
 * @example
 * 	void T32_INT1_IRQHandler(void) { . . . . }
 * @param hertz Desired frequency of interrupt in 0.1 Hz
 */
void Timer32_1_Interrupt_Init(int hertz) {
	TIMER32_1->CONTROL = 0x000000EA;			// Periodic, interrupts, /256, 32-bit
	TIMER32_1->LOAD = ((48000000 / 256) * hertz) / 10;	// (48MHz / 256) / 187500 = 1 Hz
	TIMER32_1->INTCLR = 0;						// Reset interrupt flag
	NVIC_SetPriority(T32_INT1_IRQn, 7);			// Priority 7
	NVIC_EnableIRQ(T32_INT1_IRQn);				// Enable T32.1 interrupts
}

/**
 * Configure T32.1 for 3 MHz countdown from 4294967295
 */
void Timer32_2_Counter_Init(void) {
	TIMER32_2->CONTROL = 0x0000008A;	// Free running, 32-bit, HFXTCLK / 16
	TIMER32_2->LOAD = 0xFFFFFFFF;		// Max count
	TIMER32_2->INTCLR = 0;				// Reset interrupt flag
}
