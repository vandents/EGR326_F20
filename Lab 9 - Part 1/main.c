/********************************************
 * Author: Scott VandenToorn
 * Date: October 23, 2020
 * Program: EGR 326-903 Lab 9 - Part 1
 * Description: Write a test program that display the
 * 	status (open or closed) of the sensor on your LCD.
 *
 * Pinout:
 * 	 Hall effect switch  -  P5.4
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "Time.h"
#include "ST7735.h"

/* Function prototypes */
void Init(void);
void Motor_P5_TA2_2_Init(void);
void Potentiometer_P6_A5_Init(void);
void HallEffect_P5_Init(void);
void ST7735_WriteSpeedRPM(void);


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

	ST7735_SetRotation(3);	// Landscape

	while (1) { }
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

	Clock_48MHz_Init();
	SysTick_Init();
	ST7735_InitR(INITR_REDTAB);
	Timer32_1_Interrupt_Init(3);	// 0.3 Hz interrupt
	HallEffect_P5_Init();
	__enable_irq();					// Enable interrupts
}

/**
 * Timer32.1 ISR to update status on LCD
 */
void T32_INT1_IRQHandler(void) {
	uint16_t white = ST7735_Color565(255, 128, 128);	// Character color
	TIMER32_1->INTCLR = 0;								// Reset interrupt flag

	if (P5->IN & BIT4) {
		ST7735_DrawChar(16, 50, ' ', white, 0, 4);
		ST7735_DrawChar(38, 50, 'O', white, 0, 4);
		ST7735_DrawChar(60, 50, 'P', white, 0, 4);
		ST7735_DrawChar(82, 50, 'E', white, 0, 4);
		ST7735_DrawChar(104, 50, 'N', white, 0, 4);
		ST7735_DrawChar(126, 50, ' ', white, 0, 4);
	} else {
		ST7735_DrawChar(16, 50, 'C', white, 0, 4);
		ST7735_DrawChar(38, 50, 'L', white, 0, 4);
		ST7735_DrawChar(60, 50, 'O', white, 0, 4);
		ST7735_DrawChar(82, 50, 'S', white, 0, 4);
		ST7735_DrawChar(104, 50, 'E', white, 0, 4);
		ST7735_DrawChar(126, 50, 'D', white, 0, 4);
	}
}

/**
 * Initialize hall effect inputs on P5.4 - 5.5
 */
void HallEffect_P5_Init(void) {
	P5->SEL0 &= ~0x10;
	P5->SEL1 &= ~0x10;					// P5.4 - 5.5 to GPIO
	P5->DIR &= ~0x10;					// Input
	P5->REN |= 0x10;					// Enable pull resistors
	P5->OUT |= 0x10;					// Pull up selected
}
