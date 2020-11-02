/********************************************
 * Author: Scott VandenToorn
 * Date: October 23, 2020
 * Program: EGR 326-903 Lab 8
 * Description: Configure a timer for 1 second
 * 	intervals. Also configure an external pushbutton
 * 	to start and stop your timer. Your LED display
 * 	will track your timer with updates every second,
 * 	up to 100 seconds on the FIRST 4 digit display.
 * 	Every time the button is pushed, the SECOND 4
 * 	digits will increment (starting at 0000),
 * 	indicating the number of times the button was
 * 	pressed.
 *
 * Pinout:
 *		  CLK  -  P2.3
 * DIN (SIMO)  -  P2.4
 *  CS (GPIO)  -  P2.5
 *	   Button  -  P6.6
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "Time.h"
#include "SevenSeg.h"

/* Function prototypes */
void Init(void);
void Button_P6_Init(void);

/* Global variables */
volatile uint8_t Button_Pressed = 0;
volatile int Button_Count = 0;
volatile int Second_Count = 0;


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

	while(1) {
    	if (Button_Pressed) {
    		Button_Count++;				// Increment button count
    		Button_Pressed = 0;			// Reset button flag
    		TIMER32_1->CONTROL ^= 0x20;	// Toggle timer

    		// Update display
    		SevenSeg_WriteInteger(0, 3, Second_Count);
    		SevenSeg_WriteInteger(4, 7, Button_Count);
    	}
	}
}


/**
 * Initializations
 */
void Init(void) {
    // Terminate all IO pins
    P1->DIR |= 0xFF; P1->OUT = 0;
    P2->DIR |= 0xFF; P2->OUT = 0;
    P3->DIR |= 0xFF; P3->OUT = 0;
    P4->DIR |= 0xFF; P4->OUT = 0;
    P5->DIR |= 0xFF; P5->OUT = 0;
    P6->DIR |= 0xFF; P6->OUT = 0;
    P7->DIR |= 0xFF; P7->OUT = 0;
    P8->DIR |= 0xFF; P8->OUT = 0;
    P9->DIR |= 0xFF; P9->OUT = 0;
    P10->DIR |= 0xFF; P10->OUT = 0;

	Clock_48MHz_Init();
	SysTick_Init();
	SevenSeg_P2_UCA1_Init();
	Button_P6_Init();
	Timer32_1_Interrupt_Init(1);	// 1 Hz interrupt
	__enable_irq();					// Enable interrupts
}

/**
 * Port 6 pin initializations (button)
 */
void Button_P6_Init(void) {
	P6->SEL0 &= ~BIT6;
	P6->SEL1 &= ~BIT6;	// P6.6 to GPIO
	P6->DIR &= ~BIT6;	// P6.6 to input
    P6->REN |= BIT6;	// Enable pull-up resistors
    P6->OUT |= BIT6;	// Pull-up selected
    P6->IES |= BIT6;	// Trigger on high to low
    P6->IE |= BIT6;		// Enable interrupt for P6.6
    P6->IFG = 0;		// Reset interrupt flag
    NVIC_EnableIRQ(PORT6_IRQn);	// Enable interrupts on NVIC
}

/**
 * Button interrupt
 */
void PORT6_IRQHandler(void) {
	if (P6->IFG & BIT6) {
		Button_Pressed = 1;
		P6->IFG &= 0;
	}
}

/**
 * Timer32.1 ISR for incrementing seconds
 */
void T32_INT1_IRQHandler(void) {
	TIMER32_1->INTCLR = 0;	// Reset interrupt flag
	// Increment second count if less than 100, else reset
	Second_Count = Second_Count < 100 ? ++Second_Count : 1;
	SevenSeg_WriteInteger(0, 3, Second_Count);
}
