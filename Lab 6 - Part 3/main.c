/********************************************
 * Author: Scott VandenToorn
 * Date: October 9, 2020
 * Program: EGR 326-903 Lab 6 - Part 3
 * Description: Count sequence.
 *
 * Pinout:
 *  	SCK  -  P9.5
 *     	SDA  -  P9.7
 *       AO  -  P9.2
 *      RST  -  P9.3
 *       CS  -  P9.4
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "SysTick.h"
#include "ST7735.h"

/* Function prototypes */
void Clock_Init48MHz(void);
void DrawCount(int x, int y);


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Clock_Init48MHz();
	SysTick_Init();
	ST7735_InitR(INITR_REDTAB);

	while(1) {
		DrawCount(4, 4);		// Top left
		DrawCount(104, 4);		// Top right
		DrawCount(104, 128);	// Bottom right
		DrawCount(4, 128);		// Bottom left
		DrawCount(52, 64);		// Middle
		SysTick_Delay_ms(3000);	// Delay 3 seconds
	}
}


/**
 * Uses DrawChar to count from 0 to 9 in 0.5 second intervals. x and y
 * define the top-left starting position of the character.
 * @param x Left-most position
 * @param y Highest vertical position
 */
void DrawCount(int x, int y) {
	int i;
	for (i = 0; i < 10; i++) {
		ST7735_DrawChar(x, y, i + '0', ST7735_Color565(255, 255, 255), 0, 4);
		SysTick_Delay_ms(500);
	}
	ST7735_DrawChar(x, y, ' ', ST7735_Color565(0, 0, 0), 0, 4);
}

/**
 * Initialization for clock frequencies and sources.
 * Lecture slides and Valvano library used as reference.
 *
 * Clock frequencies and sources:
 * 	HFXTCLK = 48 MHz crystal
 * 	SMCLK = HFXTCLK / 4
 * 	HSMCLK = HFXTCLK / 2
 * 	MCLK = HFXTCLK
 * 	ACLK = REFOCLK
 */
void Clock_Init48MHz(void) {
	// Configure Flash wait-state to 1 for banks 0 & 1
	FLCTL->BANK0_RDCTL =
		(FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK0_RDCTL_WAIT_1;
	FLCTL->BANK1_RDCTL =
		(FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK1_RDCTL_WAIT_1;

	// Configure HFXT to use 48MHz crystal
	PJ->SEL0 |= 0x0C;		// Configure PJ.2/3 for HFXTCLK function
	PJ->SEL1 &= ~0x0C;
	CS->KEY = CS_KEY_VAL;	// Unlock CS module
	CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;

	// Wait for the HFXT clock to stabilize
	while(CS->IFG & CS_IFG_HFXTIFG) CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

	// Set clock sources and dividers
	CS->CTL1 = 0x20000000 |	// SMCLK = HFXTCLK / 4
		0x00100000 |		// SMCLK = HFXTCLK / 2
		0x00000200 |		// ACLK source = REFOCLK
		0x00000055;			// SMCLK, HSMCLK, MCLK source = HFXTCLK

	CS->KEY = 0;
}
