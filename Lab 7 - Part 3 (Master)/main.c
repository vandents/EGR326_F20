/********************************************
 * Author: Scott VandenToorn
 * Date: October 16, 2020
 * Program: EGR 326-903 Lab 7 - Part 3 (Master)
 * Description: Use two MCU's to control stepper
 * 	motor remotely. Press (1 - 9) to select number of
 * 	steps * 4. Press buttons to increment/decrement.
 *
 * Pinout:
 *     Keypad rows  -  P2.4 - 2.7
 *  Keypad columns  -  P5.0 - 5.2
 *  	  + button  -  P6.6
 *  	  - button  -  P6.7
 *  	  UCB0 SDA  -  P1.6
 *  	  UCB0 SCL  -  P1.7
 ********************************************/


/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include "msp.h"
#include "Clock_48MHz.h"
#include "Keypad.h"
#include "SysTick.h"

/* Constants */
#define SLAVE_ADDRESS 0x48

/* Function prototypes */
void Init(void);
void Buttons_P6_Init(void);
void I2C_P1_Init(void);

/* Global variables */
volatile int8_t Cycle_Count = 0;
volatile uint16_t Step_Count = 0;


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	uint8_t key = 0;		// Code of key pressed

	Init();

	printf("\nEnter number of cycles (1 - 9)\n\n");

	while(1) {
		if (Keypad_Interrupt) {
			key = Keypad_Read();
			if (key && key < 10) {
				Cycle_Count = key;
				printf("\n%d * 4 = %d steps\n", key, key * 4);
			}

			Keypad_Enable();	// Start listening for keypresses again
		}
	}
}


/** Initializations */
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

	Clock_Init48MHz();
	SysTick_Init();
	I2C_P1_Init();			// Port 1 - I2C
	Keypad_Row_P2_Init();	// Port 2 - Keypad rows
	Keypad_Col_P5_Init();	// Port 5 - Keypad columns
	Buttons_P6_Init();		// Port 6 - Increment/decrement buttons
	__enable_irq();			// Enable interrupts
}

/**
 * Increment/decrement button initializations (Port 6)
 */
void Buttons_P6_Init(void) {
	P6->SEL0 &= ~0xC0;
	P6->SEL1 &= ~0xC0;	// P6.6 - 6.7 to GPIO
	P6->DIR &= ~0xC0;	// Input
    P6->REN |= 0xC0;	// Enable pull-up resistors
    P6->OUT |= 0xC0;	// Pull-up selected
    P6->IES |= 0xC0;	// Trigger on high to low
    P6->IE |= 0xC0;		// Enable interrupt for P6.6
    P6->IFG = 0;		// Reset interrupt flag
    NVIC_EnableIRQ(PORT6_IRQn);	// Enable interrupts on NVIC
}

/**
 * Increment/decrement button interrupt (Port 6)
 */
void PORT6_IRQHandler(void) {
	int8_t initial_cycles = Cycle_Count;

	// Increment
	if (P6->IFG & BIT6) {
		P6->IFG &= ~BIT6;
		if (Cycle_Count) {
			Cycle_Count = abs(Cycle_Count);	// Ensure positive for increment

			// Handle overshoot
			if (Step_Count + Cycle_Count * 4 > 600) {
				Cycle_Count = (600 - Step_Count) / 4;
				Step_Count = 600;
			} else Step_Count += Cycle_Count * 4;

			if (Cycle_Count) {
				EUSCI_B0->IE |= EUSCI_B_IE_TXIE0;	// Trigger TX interrupt
				printf("\nPosition:  %d steps\n", Step_Count);
			} else {
				Cycle_Count = initial_cycles;
				printf("\nMaximum reached (600 steps)\n");
			}
		}
	}

	// Decrement
	if (P6->IFG & BIT7) {
		P6->IFG &= ~BIT7;
		if (Cycle_Count) {
			Cycle_Count = -1 * abs(Cycle_Count);	// Ensure negative for decrement

			// Handle overshoot
			if (Step_Count + Cycle_Count * 4 < 0) {
				Cycle_Count = -1 * (Step_Count / 4);
				Step_Count = 0;
			} else Step_Count += Cycle_Count * 4;

			if (Cycle_Count) {
				EUSCI_B0->IE |= EUSCI_B_IE_TXIE0;	// Trigger TX interrupt
				printf("\nPosition:  %d steps\n", Step_Count);
			} else {
				Cycle_Count = initial_cycles;
				printf("\nMinimum reached (0 steps)\n");
			}
		}
	}
}

/**
 * Configure UCB0 as I2C
 */
void I2C_P1_Init(void) {
    P1->DIR &= ~BIT1;
    P1->REN = BIT1;
    P1->OUT = BIT1;
    P1->IE = BIT1;
    P1->IES = BIT1;
    P1->IFG = 0x00;
    P1->SEL0 |= BIT6 | BIT7;	// P1.6 - 1.7 as UCB0SDA and UCB0SCL

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;		// Hold EUSCI_B0 module in reset state
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_UCSSEL_2;	// Select SMCLK as EUSCI_B0 clock
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MODE_3 | EUSCI_B_CTLW0_MST | EUSCI_B_CTLW0_SYNC;
    EUSCI_B0->BRW = 0x001E;						// BITCLK = BRCLK / (UCBRx + 1) = 3 MHz / 30 = 100 kHz
    EUSCI_B0->I2CSA = SLAVE_ADDRESS;
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;	// Clear SWRST to resume operation

    NVIC->ISER[0] = 0x00100000;	// EUSCI_B0 interrupt is enabled in NVIC

    while(EUSCI_B0->CTLW0 & EUSCI_B_CTLW0_TXSTP);
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TR | EUSCI_B_CTLW0_TXSTT;
}

/**
 * EUSCI_B0 ISR
 */
void EUSCIB0_IRQHandler(void) {
    uint32_t status = EUSCI_B0->IFG;		// Get EUSCI_B0 interrupt flag
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_TXIFG0;	// Reset interrupt flag

    if (status & EUSCI_B_IFG_TXIFG0) { 		// Check if transmit interrupt occurs
        EUSCI_B0->TXBUF = Cycle_Count;		// Load data into TX buffer
        EUSCI_B0->IE &= ~EUSCI_B_IE_TXIE0;	// Disable EUSCI_B0 TX interrupt
    }
}