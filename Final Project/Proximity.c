/*
 * Proximity.c
 *
 *  Created on: Nov 20, 2020
 *      Author: scottvandentoorn
 */

#include "Proximity.h"
#include "Time.h"


/**
 * Port 4 pin initializations
 */
void Proximity_GPIO_P4_Init(void) {
    P4->SEL0 &= ~0xE0;
    P4->SEL1 &= ~0xE0;        // P4.5 - 4.7 to GPIO
    P4->DIR |= 0x60;					// P4.5 - 4.6 to output
    P4->DIR &= ~BIT7;					// P4.7 to input
    P4->REN |= BIT7;					// Enable pull resistors
    P4->OUT &= ~BIT7;					// Pull down selected
    P4->IE |= BIT7;           // Enable interrupts
    P4->IES &= ~BIT7;					// Rising edge
    P4->IFG = 0;              // Reset interrupt flag

    NVIC_SetPriority(PORT4_IRQn, 6);	// Priority 6
    NVIC_EnableIRQ(PORT4_IRQn);			// Enable Port 4 interrupt on the NVIC

    Backup_Distance = 0;
}

/**
 * Send 10us trigger signal on P4.6
 */
void Proximity_Trigger(void) {
	uint16_t i;
    P4->OUT |= BIT6;
    for (i = 0; i < 480; i++);	// 10 us delay @ 48 MHz
    P4->OUT &= ~BIT6;
}

/**
 * Port 4 ISR.
 * Triggers on rising/falling edge of echo signal to determine distance
 */
void PORT4_IRQHandler(void) {
	static uint32_t rise = 0;	// Time of rising edge
	uint32_t current = TIMER32_2->VALUE;

	// P4.7 interrupt (echo)
	if (P4->IFG & BIT7) {
		// Calculate distance (inches) on falling edge
		if (P4->IES & BIT7) Backup_Distance = (rise - current) / 30;
		else rise = current;	// Record time of rising edge
		P4->IES ^= BIT7;		// Toggle btwn rising/falling edge trigger
	}

	P4->IFG = 0;				// Reset interrupt flag
}
