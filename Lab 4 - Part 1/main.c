/********************************************
 * Author: Scott VandenToorn
 * Date: September 25, 2020
 * Program: EGR 326-903 Lab 4 - Part 1
 * Description: Prompts the used to press a key
 * 	on a keypad interfaced with the MSP432 and
 * 	prints the pressed key on the CCS console.
 *
 * Pinout:
 * 		Rows - P2.4 - 2.7
 *		Columns - P5.0 - 5.2
 ********************************************/


/* Includes */
#include "msp.h"
#include <stdio.h>

/* Function prototypes */
void P2init(void);                      // Port 2 pin initializations (rows)
void P5init(void);                      // Port 5 pin initializations (cols)
void SysTick_Init(void);                // SysTick timer initialization
void SysTick_delay_ms(uint16_t delay);  // SysTick delay (ms)
uint8_t Read_Keypad(void);              // Keypad scan subroutine
void Print_Keys(uint8_t Key);           // Key print function

/* Global variables */
volatile uint8_t KeypadPressed = 0;


/* Main */
void main(void) {
    // Key pressed
    uint8_t Key = 0;

    // Stop watchdog timer
    WDT_A->CTL = 0x5A80;

	// Initializations
	P5init();
	P2init();
	SysTick_Init();
    __enable_irq();

	printf("Press a key\n");

	while(1) {
		if (KeypadPressed) {
			// Read keypad
			Key = Read_Keypad();
			// Print key if one was pressed
			if (Key) Print_Keys(Key);

			// Reset interrupt flags
			P2->IFG = 0;
			KeypadPressed = 0;
			// Enable interrupts
		    P2->IE |= 0xF0;
		}

		// Delay while loop
		SysTick_delay_ms(5);
	}
}


/********************************************
 * Keypad
 ********************************************/

/* Port 2 ISR */
void PORT2_IRQHandler(void) {
	if (P2->IFG & 0xF0) {
		KeypadPressed = 1;
	    // Disable port 2 interrupts
		// Reset in main
		P2->IE &= ~0xF0;
	}
}

/* Initialize pins for rows */
void P2init(void) {
	/* P2.4 - 2.7 */
	// GPIO
    P2->SEL0 &= ~0xF0;
    P2->SEL1 &= ~0xF0;
    // Input
    P2->DIR &= ~0xF0;
    // Enable pull resistors
    P2->REN |= 0xF0;
    // Pull up selected
    P2->OUT |= 0xF0;
    // Enable interrupts
    P2->IE |= 0xF0;
    // Trigger on falling edge
    P2->IES |= 0xF0;
    // Reset interrupt flag
    P2->IFG = 0;

    // Enable Port 2 interrupt on the NVIC
    NVIC_EnableIRQ(PORT2_IRQn);
}

/* Initialize pins for columns */
void P5init(void) {
	/* P5.0 - 5.2 */
	// GPIO
    P5->SEL0 &= ~0x07;
    P5->SEL1 &= ~0x07;
    // Output
    P5->DIR |= 0x07;
    // Output low
    P5->OUT &= ~0x07;
}

/*
 * Keypad scan subroutine
 * @return Number corresponding to key pressed
 */
uint8_t Read_Keypad(void) {
    uint8_t col = 0, row = 0;

    // Test each column
    for (col = 0; col < 3; col++) {
    	// Set columns to inputs
        P5->DIR &= ~0x07;
        // Set column x to output
        P5->DIR |= BIT(col);
        // Set column x to LOW
        P5->OUT &= ~BIT(col);

        // 10 ms delay
        SysTick_delay_ms(10);

        // Read rows
        row = P2->IN & 0xF0;

        // Wait until keypad is released
        while(!(P2->IN & BIT4) | !(P2->IN & BIT5) | !(P2->IN & BIT6) | !(P2->IN & BIT7));

        // If one of the inputs is low, some key is pressed
        if (row != 0xF0) break;
    }

    // Columns to output low
    P5->DIR |= 0x07;
    P5->OUT &= ~0x07;

    switch(row) {
    	// Key in row 0, E => 1110
    	case 0xE0: return col + 1;
    	// Key in row 1, D => 1101
    	case 0xD0: return col + 4;
    	// Key in row 2, B => 1011
    	case 0xB0: return col + 7;
    	// Key in row 3, 7 => 0111
    	case 0x70: return col + 10;
    	default: return 0;
    }
}

/* Function to log key to console */
void Print_Keys(uint8_t Key) {
    printf("\nYou pressed: ");

    if (Key <= 9) printf("%d\n", Key);
    else if (Key == 10) printf("*\n");
    else if (Key == 11)	printf("0\n");
    else if (Key == 12) printf("#\n");
    else printf("Error\n");
}


/********************************************
 * SysTick
 ********************************************/

/* SysTick Timer initialization */
void SysTick_Init(void) {
	// Disable SysTick
    SysTick->CTRL = 0;
    // Max reload value
    SysTick->LOAD = 0x00FFFFFF;
    // Any write to current clears it
    SysTick->VAL = 0;
    // Enable SysTick, 3MHz, no interrupts
    SysTick->CTRL = 0x00000005;
}

/* SysTick delay milliseconds */
void SysTick_delay_ms(uint16_t delay) {
	// Delay for 1ms per delay value
    SysTick->LOAD = ((delay * 3000) - 1);
    // Any write to CVR clears it
    SysTick->VAL = 0;
    // Wait for flag to be SET
    while((SysTick->CTRL & 0x10000) == 0);
}
