/********************************************
 * Author: Scott VandenToorn
 * Date: October 16, 2020
 * Program: EGR 326-903 Lab 7 - Part 3 (Slave)
 * Description: Use two MCU's to control stepper
 * 	motor remotely. Press (1 - 9) to select number of
 * 	steps * 4. Press buttons to increment/decrement.
 *
 * Pinout:
 *	 A (stepper)  -  P7.6
 * 	A' (stepper)  -  P7.7
 * 	 B (stepper)  -  P7.4
 * 	B' (stepper)  -  P7.5
 *		UCB0 SDA  -  P1.6
 *		UCB0 SCL  -  P1.7
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "Clock_48MHz.h"
#include "Stepper.h"

/* Constants */
#define SLAVE_ADDRESS 0x48

/* Function prototypes */
void Init(void);
void I2C_P1_Init(void);


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

    while(1) {
    	if (Stepper_Interrupt) {
    		Stepper_Update();
    		Stepper_Interrupt = 0;
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

	Clock_Init48MHz();
	I2C_P1_Init();				// Initialize I2C
	Stepper_Timer32_1_Init();	// Initialize T32.1 for stepper
	Stepper_P7_Init();			// Initialize stepper GPIO
	__enable_irq();				// Enable interrupts
}

/**
 * Configure UCB0 as I2C
 */
void I2C_P1_Init(void) {
    P1->SEL0 |= BIT6 | BIT7;	// P1.6 and P1.7 as UCB0SDA and UCB0SCL

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;		// Hold EUSCI_B0 module in reset state
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MODE_3|EUSCI_B_CTLW0_SYNC;
    EUSCI_B0->I2COA0 = SLAVE_ADDRESS | EUSCI_B_I2COA0_OAEN;
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;	// Clear SWRST to resume operation
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_RXIFG0;		// Clear EUSCI_B0 RX interrupt flag
    EUSCI_B0->IE |= EUSCI_B_IE_RXIE0;			// Enable EUSCI_B0 RX interrupt

    NVIC->ISER[0] = 0x00100000; 				// EUSCI_B0 interrupt is enabled in NVIC
}

/**
 * EUSCI_B0 ISR
 */
void EUSCIB0_IRQHandler(void) {
	uint32_t status = EUSCI_B0->IFG;			// Get EUSCI_B0 interrupt flag
	EUSCI_B0->IFG  &=~ EUSCI_B_IFG_RXIFG0;		// Reset interrupt flag

    if (status & EUSCI_B_IFG_RXIFG0) {			// Check if receive interrupt occurs
    	int8_t Cycle_Count = EUSCI_B0->RXBUF;	// Read RX buffer
    	Stepper_Step += Cycle_Count * 4;
	}
}
