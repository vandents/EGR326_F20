/********************************************
 * Author: Scott VandenToorn
 * Date: October 16, 2020
 * Program: EGR 326-903 Lab 7 - Part 1
 * Description: Test for MX1508 motor driver
 * 	and X27-168 bipolar stepper motor.
 *
 * Pinout:
 *	 A (stepper)  -  P7.6
 * 	A' (stepper)  -  P7.7
 * 	 B (stepper)  -  P7.4
 * 	B' (stepper)  -  P7.5
 *		  Button  -  P6.6
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "SysTick.h"
#include "Clock_48MHz.h"
#include "Stepper.h"

/* Function prototypes */
void Init(void);
void Button_P6_Init(void);

/* Global variables */
volatile uint8_t Button_Pressed = 0;
volatile int8_t Increment = 10;


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

    while(1) {
    	if (Stepper_Interrupt) {
    		Stepper_Update();			// Update stepper position
    		Stepper_Interrupt = 0;		// Reset flag
    	}

    	if (Button_Pressed) {
    		Stepper_Step += Increment;	// Increment step
    		Stepper_StepsMoved = 0;
    		Increment *= -1;			// Toggle increment
    		Button_Pressed = 0;			// Reset flag
    	}
    }
}


/**
 * Initialization
 */
void Init(void) {
	Clock_Init48MHz();
	Stepper_P7_Init();
	SysTick_Init();
	Stepper_Timer32_1_Init();
	Button_P6_Init();
	__enable_interrupts();
}

/**
 * Button interrupt
 */
void PORT6_IRQHandler(void) {
	if (P6->IFG & BIT6 && !Stepper_Zero) {
		Button_Pressed = 1;
		P6->IFG &= 0;
	}
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
