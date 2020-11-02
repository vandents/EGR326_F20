/********************************************
 * Author: Scott VandenToorn
 * Date: September 25, 2020
 * Program: EGR 326-903 Lab 4 - Part 3
 * Description: Merging keypad and rotary encoder.
 *
 * Pinout:
 *         Red LED  -  P2.0
 *        Blue LED  -  P2.2
 *     Keypad rows  -  P2.4 - 2.7
 *    CLK (rising)  -  P3.0
 *   CLK (falling)  -  P3.5
 * 	            DT  -  P3.6
 * 		Pushbutton  -  P3.7
 *  Keypad columns  -  P5.0 - 5.2
 *
 * state:
 * 		0  =>  get range min
 *		1  =>  get range max
 *		2  =>  read encoder/update LEDs
 ********************************************/


/* Includes */
#include "msp.h"
#include <stdio.h>

/* Function prototypes */
void init(void);
void P2init(void);                      // Port 2 pin initializations (rows)
void P5init(void);                      // Port 5 pin initializations (cols)
void P3init(void);                      // Port 3 pin initializations (encoder)
void SysTick_Init(void);                // SysTick timer initialization
void SysTick_delay_ms(uint16_t delay);  // SysTick delay (ms)
uint8_t Read_Keypad(void);              // Keypad scan subroutine
void Print_Keys(uint8_t Key);           // Key print function
void off(void);
void red(void);
void blue(void);
void updateLED(void);

/* Global variables */
volatile uint8_t KeypadPressed = 0;
volatile uint8_t ButtonPressed = 0;
volatile uint32_t range_min = 0;
volatile uint32_t range_max = 0;
volatile uint32_t CW_count = 0;
volatile uint32_t CCW_count = 0;
volatile uint8_t state = 0;


/* Main */
void main(void) {
    WDT_A->CTL = 0x5A80;	// Stop watchdog timer
    uint8_t Key = 0;		// Code of key pressed
    init();					// Initializations

    printf("\nEnter min (1-9):\n");

	while(1) {
		if (KeypadPressed && state < 2) {
			Key = Read_Keypad();	// Read keypad

			if (Key && Key < 10) {	// Only accept numbers 1 - 9
				if (state == 0) {
					Print_Keys(Key);
					range_min = Key;
					state = 1;
					printf("Enter max (%u-9):\n", range_min);
				} else if (state == 1 && Key >= range_min) {
					Print_Keys(Key);
					range_max = Key;
					state = 2;
					printf("You may now rotate the encoder\n\n");
				}
			}

			P2->IFG = 0;		// Reset interrupt flags
			KeypadPressed = 0;	// Reset keypad flag
		    P2->IE |= 0xF0;		// Enable interrupts on rows
		}

		if (ButtonPressed) {
			off();	// Turn off LEDs

			// Reset counts, state, and button flag
			CW_count = 0;
			CCW_count = 0;
			state = 0;
			ButtonPressed = 0;

		    printf("\n\nEnter min (1-9):\n");
		}

		SysTick_delay_ms(5);	// Delay while loop
	}
}


/********************************************
 * Initialization
 ********************************************/

/* Initializations */
void init(void) {
	P5init();		// Keypad cols
	P2init();		// Keypad rows
	P3init();		// Encoder
	SysTick_Init();	// SysTick timer
    __enable_irq();	// Enable interrupts
}

/* Encoder */
void P3init(void) {
    P3->SEL0 &= ~0xE1;
    P3->SEL1 &= ~0xE1;	// GPIO 					(0, 5, 6, 7)
    P3->DIR &= ~0xE1;	// Input 					(0, 5, 6, 7)
    P3->REN |= 0xE1;	// Enable pull resistors	(0, 5, 6, 7)
    P3->OUT |= 0x81;	// Pull up 					(0, 7)
    P3->OUT &= ~0x60;	// Pull down 				(5, 6)

    P3->IE |= 0xA1;		// Enable interrupts 		(0, 5, 7)
    P3->IES &= ~0x01;	// Rising edge 				(0)
    P3->IES |= 0xA0;	// Falling edge 			(5, 7)
    P3->IFG = 0;		// Reset interrupt flag

    // Enable Port 3 interrupt on the NVIC
    NVIC_EnableIRQ(PORT3_IRQn);
}

/* Keypad rows, LEDs */
void P2init(void) {
	/* P2.4 - 2.7 (Keypad rows) */
    P2->SEL0 &= ~0xF5;
    P2->SEL1 &= ~0xF5;	// GPIO
    P2->DIR &= ~0xF0;	// Input
    P2->REN |= 0xF0;	// Enable pull resistors
    P2->OUT |= 0xF0;	// Pull up selected
    P2->IE |= 0xF0;		// Enable interrupts
    P2->IES |= 0xF0;	// Trigger on falling edge
    P2->IFG = 0;		// Reset interrupt flag

    /* P2.0, P2.2 (red/blue LEDs) */
    P2->DIR |= 0x05;	// Output
    off();				// Turn off LEDs

    // Enable Port 2 interrupt on the NVIC
    NVIC_EnableIRQ(PORT2_IRQn);
}

/* Keypad columns */
void P5init(void) {
	/* P5.0 - 5.2 */
    P5->SEL0 &= ~0x07;
    P5->SEL1 &= ~0x07;	// GPIO
    P5->DIR |= 0x07;	// Output
    P5->OUT &= ~0x07;	// Output LOW
}


/********************************************
 * Rotary encoder
 ********************************************/

/* Port 3 ISR (encoder) */
void PORT3_IRQHandler(void) {
	if (state == 2) {
		uint8_t DT = P3->IN & BIT6;	// Read DT pin

		// Rising edge of CLK
		if (P3->IFG & BIT0 && P3->IN & BIT0) {
			// Increment direction count
			if (DT) ++CCW_count;
			else ++CW_count;
//			printf("\nRise\tCW: %u\tCCW: %u", CW_count, CCW_count);
			updateLED();	// Turn on/off LEDs
		}

		// Falling edge of CLK
		else if (P3->IFG & BIT5 && !(P3->IN & BIT5)) {
			// Increment direction count
			if (DT) ++CW_count;
			else ++CCW_count;
//			printf("\nFall\tCW: %u\tCCW: %u", CW_count, CCW_count);
			updateLED();	// Turn on/off LEDs
		}
//		printf("\n");
	}

	// Set button flag
	if (P3->IFG & BIT7) ButtonPressed = 1;

	P3->IFG = 0;	// Reset interrupt flag
}


/********************************************
 * Keypad
 ********************************************/

/* Port 2 ISR (keypad) */
void PORT2_IRQHandler(void) {
	// One of the row pins had an interrupt
	if (P2->IFG & 0xF0) {
		KeypadPressed = 1;
	    // Disable port 2 interrupts
		// Reset in main
		P2->IE &= ~0xF0;
	}
}

/*
 * Keypad scan subroutine
 * @return Number corresponding to key pressed
 */
uint8_t Read_Keypad(void) {
    uint8_t col = 0, row = 0;

    // Test each column
    for (col = 0; col < 3; col++) {
        P5->DIR &= ~0x07;		// Set columns to inputs
        P5->DIR |= BIT(col);	// Set column x to output
        P5->OUT &= ~BIT(col);	// Set column x to LOW

        SysTick_delay_ms(10);	// 10 ms delay for debounce

        row = P2->IN & 0xF0;	// Read rows

        // Wait until keypad is released
        while(!(P2->IN & BIT4) | !(P2->IN & BIT5) | !(P2->IN & BIT6) | !(P2->IN & BIT7));

        // If one of the inputs is low, some key is pressed
        if (row != 0xF0) break;
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

/* Function to log key to console */
void Print_Keys(uint8_t Key) {
    if (Key <= 9) printf("%d\n", Key);
    else if (Key == 10) printf("*\n");
    else if (Key == 11)	printf("0\n");
    else if (Key == 12) printf("#\n");
    else printf("Error\n");
}


/********************************************
 * LEDs
 ********************************************/

/* Turn off LEDs */
void off(void) {
    P2->OUT &= ~(BIT0 | BIT2);
}

/* Turn on red LED */
void red(void) {
    off();
    P2->OUT ^= BIT0;
}

/* Turn on blue LED */
void blue(void) {
    off();
    P2->OUT ^= BIT2;
}

/* Turn red/blue LED if within range */
void updateLED(void) {
	int cumulative = CW_count - CCW_count;

	// Turn off LEDs if out of range
	if (abs(cumulative) < range_min || abs(cumulative) > range_max) {
		off();
		return;
	}

	if (cumulative > 0) red();	// CCW
	else blue();				// CW
}


/********************************************
 * SysTick
 ********************************************/

/* SysTick Timer initialization */
void SysTick_Init(void) {
    SysTick->CTRL = 0;			// Disable SysTick
    SysTick->LOAD = 0x00FFFFFF;	// Max reload value
    SysTick->VAL = 0;			// Reset count
    SysTick->CTRL = 0x00000005;	// Enable SysTick, 3MHz, no interrupts
}

/* SysTick delay milliseconds */
void SysTick_delay_ms(uint16_t delay) {
    SysTick->LOAD = ((delay * 3000) - 1);	// Delay for 1ms per delay value
    SysTick->VAL = 0;						// Reset count
    while((SysTick->CTRL & 0x10000) == 0);	// Wait for flag to be SET
}
