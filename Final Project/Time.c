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
 * 	MCLK = HFXTCLK = 48 MHz
 * 	SMCLK = HFXTCLK / 4 = 12 MHz
 * 	HSMCLK = HFXTCLK / 2 = 24 MHz
 * 	ACLK = REFOCLK / 4 = 8192 Hz
 */
void Clock_Frequencies_Init(void) {
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
		0x02000000 |		// ACLK = REFOCLK / 4
		0x00100000 |		// HSMCLK = HFXTCLK / 2
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
	SysTick->LOAD = delay * 48 - 1;			// Delay for 1us per delay value
	SysTick->VAL = 0;				 		// Any write to CVR clears it
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}


/**********************************************************
 * Timer32
 **********************************************************/

/**
 * Configure T32.1 for periodic interrupt
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
 * Configure T32.1 for 187500 Hz countdown from 4294967295
 */
void Timer32_2_Counter_Init(void) {
	TIMER32_2->CONTROL = 0x0000008A;	// Free running, 32-bit, HFXTCLK / 256
	TIMER32_2->LOAD = 0xFFFFFFFF;		// Max count
	TIMER32_2->INTCLR = 0;				// Reset interrupt flag
}


/**********************************************************
 * Timer A
 **********************************************************/

/**
 * Configure TA0.0 for periodic interrupt
 * @example
 * 	void TA0_0_IRQHandler(void) { . . . . }
 * @param seconds Period of interrupt in seconds
 */
void TimerA0_0_Interrupt_s_Init(uint16_t seconds) {
	// 0000 0001 1101 0010
	// ACLK, /8, up mode, interrupt enabled
	TIMER_A0->CTL = 0x01D0;					// 8192 / 8 = 1024 Hz
	TIMER_A0->CCTL[0] = 0x0010;				// Enable capture/compare interrupt
	TIMER_A0->CCR[0] = seconds * 1024 - 1;	// Set period
	NVIC_SetPriority(TA0_0_IRQn, 7);		// Priority 7
	NVIC_EnableIRQ(TA0_0_IRQn);				// Enable TA0.0 interrupts
}

/**
 * Configure TA1.0 for periodic interrupt
 * @example
 * 	void TA1_0_IRQHandler(void) { . . . . }
 * @param millis Period of interrupt in milliseconds (nearly)
 */
void TimerA1_0_Interrupt_ms_Init(uint16_t millis) {
	// 0000 0001 1101 0010
	// ACLK, /8, up mode, interrupt enabled
	TIMER_A1->CTL = 0x01D0;					// 8192 / 8 = 1024 Hz
	TIMER_A1->CCTL[0] = 0x0010;				// Enable capture/compare interrupt
	TIMER_A1->CCR[0] = millis - 1;			// Set period
	NVIC_SetPriority(TA1_0_IRQn, 7);		// Priority 7
	NVIC_EnableIRQ(TA1_0_IRQn);				// Enable TA1.0 interrupts
}
