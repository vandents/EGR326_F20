/*
 * Motor.c
 *
 *  Created on: Nov 19, 2020
 *      Author: scottvandentoorn
 */

#include "Motor.h"


/**
 * Initialize motor PWM (P5.7) and potentiometer ADC (P6.0)
 */
void Motor_PWM_ADC_Init(void) {
	Motor_HallEffect_P5_Init();
	Motor_P5_TA2_2_Init();
	Motor_Potentiometer_P6_A5_Init();
}


/********************************************
 * Potentiometer ADC
 ********************************************/

/**
 * Initialize ADC on P6.0
 */
void Motor_Potentiometer_P6_A5_Init(void) {
	P6->SEL0 |= 0x01;
	P6->SEL1 |= 0x01;					// P6.0 to ADC14 pin function

	// 01000100 11010110 00110011 10010000
	// /4, /4, ACLK, repeat-sequence-of-channels, 32 cycle sample, auto, ON
	// ADC14CLK = 8192 / 4 / 4 = 512 Hz, conversion freq = 512 / 32 = 16 Hz
	ADC14->CTL0 = 0x44D63390;
	ADC14->CTL1 = 0x00040002;			// MEM5, low-power
	ADC14->MCTL[5] = 0x8F;				// A5 to input channel
	ADC14->IER0 |= 0x20;				// Enable A5 interrupt
	ADC14->CTL0 |= 0x03;				// Start conversion

	NVIC_SetPriority(ADC14_IRQn, 7);	// Priority 7
	NVIC_EnableIRQ(ADC14_IRQn);			// Enable ADC14 interrupts
}


/********************************************
 * Motor PWM
 ********************************************/

/**
 * Initialize PWM on P5.7 using TimerA2.2
 */
void Motor_P5_TA2_2_Init(void) {
	P5->SEL0 |= 0x80;
	P5->SEL1 &= ~0x80;				// P5.7 to TimerA2.2
	P5->DIR |= 0x80;				// Output

	TIMER_A2->CCR[0] = 1800;		// Period, 15 kHz
	TIMER_A2->CCR[2] = 0;			// Duty cycle
	TIMER_A2->CCTL[2] = 0xE0;		// Reset/set mode
	TIMER_A2->CTL = 0x0216;			// 10 0001 0100
}


/********************************************
 * Hall-effect switches
 ********************************************/

/**
 * Initialize hall effect inputs on P5.4 - 5.5
 */
void Motor_HallEffect_P5_Init(void) {
	P5->SEL0 &= ~0x30;
	P5->SEL1 &= ~0x30;					// P5.4 - 5.5 to GPIO
	P5->DIR &= ~0x30;					// Input
	P5->REN |= 0x30;					// Enable pull resistors
	P5->OUT |= 0x30;					// Pull up selected
	P5->IES &= ~0x30;					// Trigger on rising edge
	P5->IE |= 0x30;						// Enable interrupts

	NVIC_SetPriority(PORT5_IRQn, 7);	// Priority 7
	NVIC_EnableIRQ(PORT5_IRQn);			// Enable Port 5 interrupts

	Motor_Spinning = 0;
	RPM = 0;
	Speed = 0;
}

/**
 * Port 5 ISR
 * Record time between interrupts to calculate speed
 */
void PORT5_IRQHandler(void) {
	// Period between interrupts on P5.4 - 5.5
	static short period_4 = 0, period_5 = 0;
	// Time of last interrupt on P5.4 - 5.5
	static uint32_t last_4 = 4294967295, last_5 = 4294967295;
	// Current value of Timer32.2
	uint32_t current = TIMER32_2->VALUE;

	if (P5->IFG & BIT4) {
		period_4 = last_4 - current;
		last_4 = current;
		Motor_Spinning = 1;

		// RPM = (60 sec * 187500 * 2 / (14 magnets * 20 gear ratio)) / sum of periods
		RPM = 80357.0 / (period_4 + period_5);
		// MPH = (71 mph / 0.36 sec / 60 sec) * RPM
		Speed = 0.426 * RPM;
	}

	if (P5->IFG & BIT5) {
		period_5 = last_5 - current;
		last_5 = current;
		Motor_Spinning = 1;

		// RPM = (60 sec * 187500 * 2 / (14 magnets * 20 gear ratio)) / sum of periods
		RPM = 80357.0 / (period_4 + period_5);
		// MPH = (71 mph / 0.36 sec / 60 sec) * RPM
		Speed = RPM * 0.426;
	}

	P5->IFG = 0;	// Reset Port 5 interrupt flag
}
