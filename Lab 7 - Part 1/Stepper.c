/*
 * Stepper.c
 *
 *  Created on: Oct 27, 2020
 *      Author: scottvandentoorn
 */

#include "Stepper.h"
#include <stdlib.h>


/**
 * Initialize stepper motor driver pins (Port 7)
 *
 * Pinout:
 * 	A	-	P7.6
 * 	A'	-	P7.7
 * 	B	-	P7.4
 * 	B'	-	P7.5
 */
void Stepper_P7_Init(void) {
	P7->SEL0 &= ~0xF0;
    P7->SEL1 &= ~0xF0;	// P7.4 - 7.7 to GPIO
    P7->DIR |= 0xF0;	// Output
    P7->OUT |= 0x50;	// P7.4, 7.6 output HIGH
	P7->OUT &= ~0xA0;	// P7.5, 7.7 output LOW

	Stepper_Interrupt = 0;
	Stepper_StepsMoved = 0;

    // Make stepper zero
    Stepper_Step = 600;
    Stepper_Zero = 1;
}

/**
 * Move motor one step
 *
 * Step sequence:
 * 	B	A
 * 	1	1
 * 	1	0
 * 	0	0
 * 	0	1
 */
void Stepper_Update(void) {
	static uint8_t state = 0b11;
	static uint16_t current_step = 0;
	static uint16_t step_gap = 0;
	uint8_t steady_state = current_step == Stepper_Step;
	uint8_t right_turn = current_step < Stepper_Step;

	if (Stepper_StepsMoved == 0) {
		step_gap = abs(Stepper_Step - current_step);
	}

	// Don't proceed if step count met
	if (steady_state) {
		Stepper_StepsMoved = 0;
		if (!Stepper_Zero) return;
	}

	// Turn on pins accordingly and update next state
	switch(state) {
		case 0b11:
			P7->OUT |= BIT4 | BIT6;
			P7->OUT &= ~(BIT5 | BIT7);
			state = right_turn ? 0b01 : 0b10;
			break;
		case 0b10:
			P7->OUT |= BIT4 | BIT7;
			P7->OUT &= ~(BIT5 | BIT6);
			state = right_turn ? 0b11 : 0b00;
			break;
		case 0b00:
			P7->OUT |= BIT5 | BIT7;
			P7->OUT &= ~(BIT4 | BIT6);
			state = right_turn ? 0b10 : 0b01;
			break;
		case 0b01:
			P7->OUT |= BIT5 | BIT6;
			P7->OUT &= ~(BIT4 | BIT7);
			state = right_turn ? 0b00 : 0b11;
			break;
		default: break;
	}

	// Record current step position
	current_step += right_turn ? 1 : -1;

	// Update step frequency
	Stepper_UpdateFrequency(++Stepper_StepsMoved, step_gap);

	// Logic to end zeroing
	if (Stepper_Zero && steady_state) {
		Stepper_Zero = Stepper_Step ? 1 : 0;
		current_step = Stepper_Step ? 650 : 0;
		Stepper_Step = 0;
	}
}

/**
 * Set Timer32.1 load for step frequency
 * @param steps Current number of steps rotated
 * @param gap Number of steps between start and stop positions
 */
void Stepper_UpdateFrequency(uint16_t steps, uint16_t gap) {
	int load = steps < gap / 2 ? 400 - steps * 3 : 400 - (gap - steps) * 3;
	if (load < 110) load = 110;
	if (gap - steps < 10) load = 400;
	TIMER32_1->BGLOAD = load;
}

/**
 * Configure T32.1 for 469 Hz interrupt
 */
void Stepper_Timer32_1_Init(void) {
	TIMER32_1->CONTROL = 0x00E8;	// Periodic, interrupts, /256, 16-bit
	TIMER32_1->LOAD = 400;			// (48 MHz / 256) / 400 = 469 Hz
	TIMER32_1->INTCLR = 0x0;		// Reset interrupt flag
	NVIC_EnableIRQ(T32_INT1_IRQn);	// Enable interrupts on NVIC
}

/**
 * Timer32.1 ISR
 */
void T32_INT1_IRQHandler(void) {
	Stepper_Interrupt = 1;		// Set flag, reset in main
	TIMER32_1->INTCLR = 0x0;	// Reset interrupt flag
}
