/********************************************
 * Author: Scott VandenToorn
 * Date: September 11, 2020
 * Program: EGR 326-10 Lab 2 - Part 2
 * Description: Sequencing colors of a RGB LED
 * using a timer interrupt and pushbutton
 * Pinout:
 *      P2.0        Red LED
 *      P2.1        Green LED
 *      P2.2        Blue LED
 *      P6.6        Button 1
 ********************************************/

#include "msp.h"
#include <stdio.h>

void updateLED(void);
void off(void);
void red(void);
void green(void);
void blue(void);
void initPort2(void);
void initPort6(void);
void SysTick_interrupt_init();

volatile uint32_t SysTick_timeout;
volatile uint32_t buttonPressed;
volatile int LED_state;
volatile uint32_t LED_order;
unsigned char BACKGROUND = 0;


void main() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Initializations
    initPort2();
	initPort6();
	SysTick_timeout = 0;
	buttonPressed = 0;
	LED_state = 0;
	LED_order = 0;
	SysTick_interrupt_init();
	__enable_irq();

	while(1) {
		// Button is pressed and SysTick timed out
		if (P6->IN & BIT6 && SysTick_timeout) {
			updateLED();

			// Determine next LED state
			LED_state += LED_order ? 1 : -1;
			if (LED_state > 2) LED_state = 0;
			else if (LED_state < 0) LED_state = 2;

			SysTick_timeout = 0;
		}
		BACKGROUND = 1;
	}
}


/********************************************
 * LEDs
 ********************************************/

/* Turn on current LED */
void updateLED() {
	switch (LED_state) {
		case 0: red(); break;
		case 1: green(); break;
		case 2: blue(); break;
		default: break;
	}
}

/* Port 2 pin initializations (LEDs) */
void initPort2() {
    // P2.0 - 2.2 to GPIO
    P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
    P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);
    // P2.0 - 2.2 to output
    P2->DIR |= BIT0 | BIT1 | BIT2;
    off();
}

void off() {
    P2->OUT &= ~(BIT0 | BIT1 | BIT2);
}

void red() {
    off();
    P2->OUT ^= BIT0;
}

void green() {
    off();
    P2->OUT ^= BIT1;
}

void blue() {
    off();
    P2->OUT ^= BIT2;
}


/********************************************
 * Button
 ********************************************/

/* Port 6 ISR */
void PORT6_IRQHandler() {
	// Toggle LED order
	LED_order = LED_order ? 0 : 1;
	// Reset interrupt flag
	P6->IFG &= 0;
}

/* Port 6 pin initializations (button) */
void initPort6() {
    // P6.6 to GPIO
    P6->SEL0 &= ~BIT6;
    P6->SEL1 &= ~BIT6;
    // P6.6 to input
    P6->DIR &= ~BIT6;
    // Enable internal resistor
    P6->REN |= BIT6;
    // Enable pull-up resistor (Output high)
    P6->OUT |= BIT6;
    // Set pin interrupt to trigger on high to low
    P6->IES |= BIT6;
    // Enable interrupt for P6.6
    P6->IE  |= BIT6;
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
    // Reload value for 1s interrupt delay
    SysTick->LOAD = 3000000;
    // Any write to current clears it
    SysTick->VAL = 0;
    // Enable SysTick, 3MHz, interrupts
    SysTick->CTRL = 0x00000007;
}

/* Set flag for timeout, reset in main */
void SysTick_Handler() {
	SysTick_timeout = 1;
}
