/********************************************
 * Author: Scott VandenToorn
 * Date: September 25, 2020
 * Program: EGR 326-903 Lab 4 - Part 2
 * Description: Detecting knob direction with
 * 	the rotary encoder.
 *
 * Pinout:
 *  		   CLK - P3.5
 * 	            DT - P3.6
 * 		Pushbutton - P3.7
 ********************************************/


/* Includes */
#include "msp.h"
#include <stdio.h>

/* Function prototypes */
void P3init(void);                      // Port 3 pin initializations
void SysTick_Init(void);                // SysTick timer initialization
void SysTick_delay_ms(uint16_t delay);  // SysTick delay (ms)

/* Global variables */
volatile uint32_t CW_count = 0;
volatile uint32_t CCW_count = 0;


/* Main */
void main(void) {
    // Stop watchdog timer
    WDT_A->CTL = 0x5A80;

	// Initializations
	P3init();
	SysTick_Init();
    __enable_irq();

    printf("\nPress button to display cumulative pulse count and direction\n\n");

	while(1) {
		// Delay while loop
		SysTick_delay_ms(10);
	}
}

/* Initialize pins for encoder */
void P3init(void) {
	/* P3.5 - P3.7 */
	// GPIO
    P3->SEL0 &= ~0xE0;
    P3->SEL1 &= ~0xE0;
    // Input
    P3->DIR &= ~0xE0;
    // Enable pull resistors
    P3->REN |= 0xE0;
    // Pull down selected (P3.5 - 3.6)
    P3->OUT &= ~0x60;
    // Pull up selected (P3.7)
    P3->OUT |= BIT7;

    /* P3.5, P3.7 */
    // Enable interrupts
    P3->IE |= 0xA0;
    // Trigger on rising edge (P3.5)
    P3->IES &= ~BIT5;
    // Trigger on falling edge (P3.7)
    P3->IES |= BIT7;
    // Reset interrupt flag
    P3->IFG = 0;

    // Enable Port 3 interrupts on NVIC
    NVIC_EnableIRQ(PORT3_IRQn);
}

/* Port 3 ISR (encoder) */
void PORT3_IRQHandler(void) {
	// Rising or falling edge of encoder clock
	if (P3->IFG & BIT5 && P3->IN & BIT5) {
		// Rising edge
		if (!(P3->IES & BIT7)) {
			if (P3->IN & BIT6) ++CCW_count;
			else ++CW_count;
			// Trigger on falling edge next time
			P3->IES |= BIT5;
		} else {
			if (P3->IN & BIT6) ++CW_count;
			else ++CCW_count;
			// Trigger on rising edge next time
			P3->IES &= ~BIT5;
		}
	// Button pressed
	} else 	if (P3->IFG & BIT7) {
		printf("Cumulative count: %u\n", abs(CW_count - CCW_count));
		if (CW_count > CCW_count)
			printf("       Direction: CW\n\n");
		else if (CCW_count > CW_count)
			printf("       Direction: CCW\n\n");
		else printf("       Direction: Undetermined\n\n");

		CW_count = 0;
		CCW_count = 0;
	}

	// Reset interrupt flag
	P3->IFG = 0;
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
