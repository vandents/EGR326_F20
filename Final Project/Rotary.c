/*
 * Rotary.c
 *
 *  Created on: Nov 18, 2020
 *      Author: scottvandentoorn
 */

#include "Rotary.h"


/**
 * Initialize rotary encoder GPIO
 * Pinout:
 *		   CLK  -  P3.5
 *			DT  -  P3.6
 *		Button  -  P3.7
 */
void Rotary_P3_GPIO_Init(void) {
	/* P5.5 - 5.7 to GPIO */
	P3->SEL0 &= ~0xE0;
	P3->SEL1 &= ~0xE0;					// GPIO
	P3->DIR &= ~0xE0;					// Input
	P3->REN |= 0xE0;					// Enable pull resistors
	P3->OUT &= ~0x60;					// Pull down (5, 6)
	P3->OUT |= 0x80;					// Pull up (7)

	P3->IE |= 0xA0;						// Enable interrupts (5, 7)
	P3->IES |= 0xA0;					// Falling edge (5, 7)
	P3->IFG = 0;						// Reset interrupt flag

	NVIC_SetPriority(PORT3_IRQn, 3);	// NVIC priority 3
	NVIC_EnableIRQ(PORT3_IRQn);			// Enable Port 3 interrupts

	// Reset flags
	Rotary_Button = 0;
	Rotary_CW = 0;
	Rotary_CCW = 0;
}

/**
 * Port 3 ISR (encoder)
 */
void PORT3_IRQHandler(void) {
	uint8_t DT = P3->IN & BIT6;		// Read DT
	uint8_t CLK = P3->IN & BIT5;	// Read CLK

	// Falling edge of CLK
	if (P3->IFG & BIT5 && !CLK) {
		// Set flag for direction turned, reset in main
		Rotary_CW = DT;
		Rotary_CCW = !DT;
	}

	// Button pressed
	if (P3->IFG & BIT7) Rotary_Button = 1;

	// Reset interrupt flag
	P3->IFG = 0;
}
