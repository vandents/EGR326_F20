/********************************************
 * Author: Scott VandenToorn
 * Date: September 11, 2020
 * Program: EGR 326-10 Lab 2 - Part 3
 * Description: Controlling the LED lighting
 * sequence using a timer interrupt and two
 * pushbutton interrupts
 * Pinout:
 *      P2.0        Red LED
 *      P2.1        Green LED
 *      P2.2        Blue LED
 *      P6.6        Button 2
 *      P6.7        Button 1
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
void SysTick_interrupt_init(void);

volatile uint32_t SysTick_timeout;
volatile uint32_t buttonPressed;
volatile int LED_state;
unsigned char BACKGROUND = 0;


void main() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Initializations
    initPort2();
	initPort6();
	SysTick_timeout = 0;
	buttonPressed = 0;
	LED_state = 0;
	SysTick_interrupt_init();
	__enable_irq();

	while(1) {
		// Button is pressed and SysTick timed out
		if (P6->IN & BIT6 && SysTick_timeout) {
			// Turn off LEDs if one is on
			if ((P2->OUT & 0b00000111) > 0) off();
			// Turn on selected LED
			else updateLED();
			SysTick_timeout = 0;
		}
		BACKGROUND = 1;
	}
}


/********************************************
 * LEDs
 ********************************************/

/* Port 2 pin initializations (LEDs) */
void initPort2() {
    // P2.0 - 2.2 to GPIO
    P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
    P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);
    // P2.0 - 2.2 to output
    P2->DIR |= BIT0 | BIT1 | BIT2;
    off();
}

/* Turn on selected LED */
void updateLED() {
	switch (LED_state) {
		case 0: red(); break;
		case 1: green(); break;
		case 2: blue(); break;
		default: break;
	}
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
 * Buttons
 ********************************************/

/* Port 6 ISR */
void PORT6_IRQHandler() {
	// Update LED color if button 1 pressed
	if (P6->IFG & BIT7) {
		LED_state++;
		if (LED_state > 2) LED_state = 0;
		else if (LED_state < 0) LED_state = 2;
		updateLED();
	}
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
    // Enable pull-up resistor
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
    // Reload value for 0.5s interrupt delay
    SysTick->LOAD = 1500000;
    // Any write to current clears it
    SysTick->VAL = 0;
    // Enable SysTick, 3MHz, interrupts
    SysTick->CTRL = 0x00000007;
}

/* Set flag for timeout, reset in main */
void SysTick_Handler() {
	SysTick_timeout = 1;
}
