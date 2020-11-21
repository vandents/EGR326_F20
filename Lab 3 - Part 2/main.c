/********************************************
 * Author: Scott VandenToorn
 * Date: September 18, 2020
 * Program: EGR 326-10 Lab 3 - Part 2
 * Description: Creating alert for proximity
 * Pinout:
 * 		P2.0		LED (TA0.1 PWM output)
 *      P4.6		trig
 *      P4.7		echo
 *
 * NOTES FOR PROJECT:
 * 	Put this chunk of code in a timer to run every 60ms
 * 		    // Reset interrupt flag
 * 		    P4->IFG = 0;
 * 		    // 10us trigger signal on P4.6
 * 		    P4->DIR |= BIT6;
 * 		    P4->OUT |= BIT6;
 * 		    SysTick_delay_us(10);
 * 		    P4->OUT &= ~BIT6;
 *
 ********************************************/


/* Includes */
#include "msp.h"
#include <stdio.h>

/* Global variables */
volatile uint32_t millis = 0;
volatile uint32_t inches = 0;

/* Function prototypes */
void initPort4(void);
void TimerA10_init(void);
void TimerA01_init(void);
void TimerA0_port_remap(void);
void update_LED_PWM(uint32_t duty);
void SysTick_Init(void);
void SysTick_delay_ms(uint16_t delay);
void SysTick_delay_us(uint16_t delay);


/* Main */
void main() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// LED PWM duty cycle
	uint32_t dutyCycle = 0;

	// Initializations
	SysTick_Init();
	TimerA0_port_remap();
	TimerA01_init();
	TimerA10_init();
    initPort4();
    __enable_irq();

	while(1) {
	    // Reset interrupt flag
//	    P4->IFG = 0;
		// 10us trigger signal on P4.6
//        P4->DIR |= BIT6;
        P4->OUT |= BIT6;
        SysTick_delay_us(10);
        P4->OUT &= ~BIT6;

        // Wait for echo signal,
        // sensor times out after 38ms
        SysTick_delay_ms(40);
        printf("%lu\n", inches);

        // Calculate LED PWM duty cycle
        if (inches == 0 || inches >= 10) dutyCycle = 0;
        else if (inches > 1) dutyCycle = inches * 10;
        else if (inches == 1) dutyCycle = 100;

        // Update LED PWM
        if (inches >= 1) update_LED_PWM(dutyCycle);
        // Extra 20 ms delay to make cycle period > 60 ms
        SysTick_delay_ms(20);
	}
}


/********************************************
 * Port 4
 ********************************************/

/* Port 4 pin initializations */
void initPort4(void) {
	// P4.6 - 4.7 to GPIO
    P4->SEL0 &= ~(BIT6 | BIT7);
    P4->SEL1 &= ~(BIT6 | BIT7);

	// P4.7 to input
    P4->DIR &= ~BIT7;
    // Enable pull resistors
    P4->REN |= BIT7;
    // Pull down selected
    P4->OUT &= ~BIT7;

    // Enable interrupt for P4.7
    P4->IE |= BIT7;
    // Trigger on rising edge
    P4->IES &= ~BIT7;
    // Reset interrupt flag
    P4->IFG = 0;
    // Enable Port 4 interrupt on the NVIC
    NVIC_EnableIRQ(PORT4_IRQn);
}

/*
 * Port 4 ISR.
 * Triggers on rising/falling edge of echo signal to determine distance
 */
void PORT4_IRQHandler(void) {
	static uint32_t previous = 0;
	uint32_t current = SysTick->VAL;

	// P4.7 interrupt
    if (P4->IFG & BIT7) {
    	// Rising edge of echo
        if (!(P4->IES & BIT7)) {
            // Record time of rising edge
            previous = current;
            // Trigger on falling edge
            P4->IES |= BIT7;

        // Falling edge of echo
        } else {
        	// Calculate inches from delay
        	// 1 * ECHO_DELAY = 0.333333 us
        	// inches = (delay - calibration) / (3 * inch conversion)
            inches = (abs(current - previous) - 50) / (3 * 148);

            // Set back to trigger on rising edge
            P4->IES &= ~BIT7;
        }
    }

    P4->IFG = 0;	// Reset interrupt flag
}


/********************************************
 * Timer A0
 ********************************************/

/* Remap P2.0 to Timer A0.1 output */
void TimerA0_port_remap(void) {
	// Unlock port mapping controller
    PMAP->KEYID = 0x2D52;
    // Map P2.0 to Timer A0.1
    P2MAP->PMAP_REGISTER0 = 20;

    // Set P2.0 to take Timer A0.1 output
    P2->DIR |= BIT0;
    P2->SEL0 |= BIT0;
    P2->SEL1 &= ~BIT0;

    // Lock port mapping controller
    PMAP->CTL = 0;
    PMAP->KEYID = 0;
}

/*
 * Timer A0.1 initialization for PWM
 */
void TimerA01_init(void) {
	// Period, (clock cycle period * (1000 - 1))
	TIMER_A0->CCR[0] = 1000 - 1;
	// 0 duty cycle
    TIMER_A0->CCR[1] = 0;
    // CCR0 reset/set mode = 7
    TIMER_A0->CCTL[1] = 0xE0;
    // Use SMCLK, count up, clear TA0R register
    TIMER_A0->CTL = 0x0214;
}

/* Sets duty cycle for LED connected to TA0.1 PWM */
void update_LED_PWM(uint32_t duty) {
    TIMER_A0->CCR[1] = duty * 10;
}


/********************************************
 * Timer A1
 ********************************************/

/*
 * Timer A1.0 initialization.
 * For timing of LED pulse
 */
void TimerA10_init(void) {
	// CCR0 interrupt enabled
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE;
    // 100Hz
    TIMER_A1->CCR[0] = 30000 - 1;
    // SMCLK, upmode
    TIMER_A1->CTL = TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR;
    // Enable Timer A1.0 interrupt on the NVIC
    NVIC_EnableIRQ(TA1_0_IRQn);
}

/*
 * Timer A1.0 ISR
 * Pulses LED when inches < 1
 */
void TA1_0_IRQHandler(void) {
	static uint8_t state = 0;

	if (inches < 1) {
		switch(state) {
			case 17:
				update_LED_PWM(0);
				state++;
				break;
			case 34:
				update_LED_PWM(100);
				state = 0;
				break;
			default:
				state++;
				break;
		}
	}

    // Reset interrupt flag
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
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

/* SysTick delay microseconds */
void SysTick_delay_us(uint16_t delay) {
	// Delay for 1us per delay value
    SysTick->LOAD = ((delay * 3) - 1);
    // Any write to CVR clears it
    SysTick->VAL = 0;
    // Wait for flag to be SET
    while((SysTick->CTRL & 0x10000) == 0);
}
