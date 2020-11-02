/*
 * Keypad.c
 *
 *  Created on: Oct 3, 2020
 *      Author: scottvandentoorn
 */

#include "Keypad.h"


/**
 * Keypad rows, P2.4 - 2.7
 */
void Keypad_Row_P2_Init(void) {
    P2->SEL0 &= ~0xF0;
    P2->SEL1 &= ~0xF0;	// GPIO
    P2->DIR &= ~0xF0;	// Input
    P2->REN |= 0xF0;	// Enable pull resistors
    P2->OUT |= 0xF0;	// Pull up selected
    P2->IES |= 0xF0;	// Trigger on falling edge

    Keypad_Enable();	// Configure interrupts

    // Enable Port 2 interrupt on the NVIC
    NVIC_EnableIRQ(PORT2_IRQn);
}

/**
 * Keypad columns, P5.0 - 5.2
 */
void Keypad_Col_P5_Init(void) {
    P5->SEL0 &= ~0x07;
    P5->SEL1 &= ~0x07;	// GPIO
    P5->DIR |= 0x07;	// Output
    P5->OUT &= ~0x07;	// Output LOW
}

/**
 * Disable keypad interrupts while reading it
 */
void Keypad_Disable(void) {
	Keypad_Interrupt = 1;
	P2->IE &= ~0xF0;	// Disable interrupts on P2.4 - 2.7
}

/**
 * Start listening for keypresses
 */
void Keypad_Enable(void) {
	P2->IFG = 0;			// Reset interrupt flags
	P2->IE |= 0xF0;			// Enable interrupts on rows
	Keypad_Interrupt = 0;	// Reset keypad flag
}

/**
 * Port 2 ISR
 */
void PORT2_IRQHandler(void) {
	// One of the Keypad row pins had an interrupt
	if (P2->IFG & 0xF0) Keypad_Disable();
}

/**
 * Keypad scan subroutine
 * @return A number (1 - 12) corresponding to key pressed
 */
uint8_t Keypad_Read(void) {
    uint8_t col = 0, row = 0;

    // Test each column
    for (col = 0; col < 3; col++) {
        P5->DIR &= ~0x07;		// Set columns to inputs
        P5->DIR |= BIT(col);	// Set column x to output
        P5->OUT &= ~BIT(col);	// Set column x to LOW

        SysTick_Delay_ms(10);	// 10 ms debounce delay

        row = P2->IN & 0xF0;	// Read rows

        // Wait until keypad is released
        while(!(P2->IN & BIT4) | !(P2->IN & BIT5) | !(P2->IN & BIT6) | !(P2->IN & BIT7));

        if (row != 0xF0) break;	// Break if input detected on row
    }

    // Column outputs to LOW
    P5->DIR |= 0x07;
    P5->OUT &= ~0x07;

    // Return keycode of key pressed
    switch(row) {
    	case 0xE0: return col + 1;	// Row 0,  E0 => 1110 0000
    	case 0xD0: return col + 4;	// Row 1,  D0 => 1101
    	case 0xB0: return col + 7;	// Row 2,  B0 => 1011 0000
    	case 0x70: return col + 10;	// Row 3,  70 => 0111 0000
    	default: return 0;
    }
}

/**
 * Logs a Keypad character to the console
 */
void Keypad_PrintKey(uint8_t key) {
    if (key <= 9) printf("%d", key);
    else if (key == 10) printf("*");
    else if (key == 11)	printf("0");
    else if (key == 12) printf("#");
    else printf("Error\n");
}
