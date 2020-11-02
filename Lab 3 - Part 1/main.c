/********************************************
 * Author: Scott VandenToorn
 * Date: September 18, 2020
 * Program: EGR 326-10 Lab 3 - Part 1
 * Description: Measuring distance using the
 * 	proximity sensor
 * Pinout:
 *      P2.6		trig
 *      P2.7		echo
 ********************************************/


/* Includes */
#include "msp.h"
#include <stdio.h>
#include <stdint.h>

/* Global variables */
volatile long millis = 0;
volatile long echo_duration_us = 0;

/* Function prototypes */
void initPort2(void);
void TA0_init(void);
void SysTick_Init(void);               	// SysTick timer initialization
void SysTick_delay_ms(uint16_t delay); 	// SysTick delay ms
void SysTick_delay_us(uint16_t delay); 	// SysTick delay us


void main() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Initializations
	SysTick_Init();
	TA0_init();
    initPort2();
    __enable_irq();

	while(1) {
		// 10us trigger signal
        P2->DIR |= BIT6;
        P2->OUT |= BIT6;
        SysTick_delay_us(9);
        P2->OUT = 0;

        // Sensor times out after 38ms
        SysTick_delay_ms(40);

        printf("Distance (cm): %d\n", echo_duration_us / 58);
        SysTick_delay_ms(2000);
	}
}


/* Port 2 pin initializations */
void initPort2() {
	// P2.6 - 2.7 to GPIO
    P2->SEL0 &= ~(BIT6 | BIT7);
    P2->SEL1 &= ~(BIT6 | BIT7);

	// P2.7 to input
    P2->DIR &= ~BIT7;
    // P2.7 pull down enabled
    P2->REN |= BIT7;
    // P2.7 initial output low
    P2->OUT &= ~BIT7;

    // Enable interrupt for P2.7
    P2->IE |= BIT7;
    // Trigger on rising edge
    P2->IES &= ~BIT7;
    // Reset interrupt flag
    P2->IFG = 0;
    // Enable Port 2 interrupt on the NVIC
    NVIC_EnableIRQ(PORT2_IRQn);
}

/*
 * Port 2 ISR.
 * Triggers on rising/falling edge of echo signal to determine
 */
void PORT2_IRQHandler(void) {
	// P2.7 interrupt
    if (P2->IFG & BIT7) {
    	// Rising edge
        if(!(P2->IES & BIT7)) {
        	// Clear timer A
            TIMER_A0->CTL |= TIMER_A_CTL_CLR;
            // Reset ms
            millis = 0;
            // Trigger on falling edge
            P2->IES |= BIT7;

        // Falling edge
        } else {
        	// Calculate echo length
        	echo_duration_us =
        		(long) (millis * 1000) +
				// 1 * R = 1/3 us
				(long) TIMER_A0->R / 3;
            // Set back to trigger on rising edge
            P2->IES &= ~BIT7;
        }

        // Reset interrupt flag
        P2->IFG = 0;
    }
}


/********************************************
 * Timer A
 ********************************************/

/* Timer A0 init */
void TA0_init(void) {
	// CCR0 interrupt enabled
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;
    // 1ms at 3MHz
    TIMER_A0->CCR[0] = 3000 - 1;
    // SMCLK, upmode
    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR;
    // Enable Timer A0 interrupt on the NVIC
    NVIC_EnableIRQ(TA0_0_IRQn);
}

/*
 * Timer A0.0 ISR. Triggered for every clock cycle
 * in SMCLK Mode counting number of pulses.
 */
void TA0_0_IRQHandler(void) {
    // Increment ms
	millis++;
    // Reset interrupt flag
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}


/********************************************
 * SysTick
 ********************************************/

/* SysTick Timer initialization */
void SysTick_Init(void) {
	// Disable SysTick
    SysTick -> CTRL = 0;
    // Max reload value
    SysTick -> LOAD = 0x00FFFFFF;
    // Any write to current clears it
    SysTick -> VAL = 0;
    // Enable SysTick, 3MHz, no interrupts
    SysTick -> CTRL = 0x00000005;
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

/* SysTick delay microseconds */
void SysTick_delay_us(uint16_t delay) {
	// Delay for 1us per delay value
    SysTick->LOAD = ((delay * 3) - 1);
    // Any write to CVR clears it
    SysTick->VAL = 0;
    // Wait for flag to be SET
    while((SysTick->CTRL & 0x10000) == 0);
}
