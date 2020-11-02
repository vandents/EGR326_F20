/********************************************
 * Author: Scott VandenToorn
 * Date: September 11, 2020
 * Program: EGR 326-10 Lab 2 - Part 1
 * Description: Controlling the blink rate of
 * an LED using pushbutton switches
 * Pinout:
 *      P5.0        LED
 *      P6.6        Button 1
 *      P6.7        Button 2
 ********************************************/

#include "msp.h"
#include <stdio.h>

void initPort5(void);
void initPort6(void);
void SysTick_interrupt_init();

volatile uint32_t SysTick_timeout;
volatile uint32_t SysTick_reload;


void main() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Initializations
	initPort5();
    initPort6();
	SysTick_timeout = 0;
	SysTick_reload = 0;
	__enable_irq();

	while(1) {
		if (SysTick_timeout) {
			P5->OUT ^= BIT0;
			SysTick_timeout = 0;
		}
	}
}


/********************************************
 * LED
 ********************************************/

/* Port 5 pin initializations (LED) */
void initPort5() {
    // P5.0 to GPIO
    P5->SEL0 &= ~BIT0;
    P5->SEL1 &= ~BIT0;
    // P5.0 to output
    P5->DIR |= BIT0;
    P5->OUT &= ~BIT0;
}


/********************************************
 * Buttons
 ********************************************/

/* Port 6 ISR */
void PORT6_IRQHandler() {
	// LED off and button 1 pressed
	if (!SysTick_reload && P6->IFG & BIT6)
		SysTick_reload = 1500000;	// 2Hz
	// Button 1
	else if (P6->IFG & BIT6)
		SysTick_reload *= 2;
	// Button 2
	else if (P6->IFG & BIT7)
		SysTick_reload /= 2;

	// Turn off if less than 2Hz
	if (SysTick_reload < 1500000) {
		SysTick_reload = 0;
		SysTick->CTRL = 0;
		P5->OUT &= ~BIT0;
	// Initialize timer with new reload value
	} else SysTick_interrupt_init();

	// Reset interrupt flag
	P6->IFG &= 0;
}

/* Port 6 pin initializations (buttons) */
void initPort6() {
    // P6.6 - 6.7 to GPIO
    P6->SEL0 &= ~(BIT6 | BIT7);
    P6->SEL1 &= ~(BIT6 | BIT7);
    // P6.6 - 6.7 to input
    P6->DIR &= ~(BIT6 | BIT7);
    // Enable internal resistor
    P6->REN |= (BIT6 | BIT7);
    // Enable pull-up resistor (Output high)
    P6->OUT |= (BIT6 | BIT7);
    // Set pin interrupt to trigger on high to low
    P6->IES |= (BIT6 | BIT7);
    // Enable interrupt for P6.6 - 6.7
    P6->IE  |= (BIT6 | BIT7);
    // Reset interrupt flag
    P6->IFG = 0;
    // Enable Port 6 interrupt on the NVIC
    NVIC_EnableIRQ(PORT6_IRQn);
}


/********************************************
 * SysTick
 ********************************************/

/* Initialize SysTick timer */
void SysTick_interrupt_init() {
	// Disable SysTick
    SysTick->CTRL = 0;
    // Reload value for interrupt delay
    SysTick->LOAD = SysTick_reload;
    // Any write to current clears it
    SysTick->VAL = 0;
    // Enable SysTick, 3MHz, interrupts
    SysTick->CTRL = 0x00000007;
}

/* Set flag for timeout, reset in main */
void SysTick_Handler() {
	SysTick_timeout = 1;
}
