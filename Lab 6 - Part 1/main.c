/********************************************
 * Author: Scott VandenToorn
 * Date: October 9, 2020
 * Program: EGR 326-903 Lab 6 - Part 1
 * Description: ST7735 LCD display test.
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

/* Function definitions */
void Clock_Init48MHz(void);
void DrawChar_Test(void);
void DrawEGR326_Test(void);


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Clock_Init48MHz();
	SysTick_Init();
	ST7735_InitR(INITR_REDTAB);

	while(1) {
		DrawChar_Test();		// Run test 1
		SysTick_Delay_ms(1000);	// Delay 1 second
		DrawEGR326_Test();		// Run test 2
	}
}


/**
 * Display multiple characters in various sizes and colors.
 * Stolen from Valvano's library.
 */
void DrawChar_Test(void) {
	ST7735_FillScreen(0);	// Set screen black

	ST7735_DrawCharS(0, 0, 'c', ST7735_Color565(255, 0, 0), 0, 1);
	ST7735_DrawCharS(6, 0, 'h', ST7735_Color565(255, 128, 0), 0, 1);
	ST7735_DrawCharS(12, 0, 'a', ST7735_Color565(255, 255, 0), 0, 1);
	ST7735_DrawCharS(18, 0, 'r', ST7735_Color565(128, 255, 0), 0, 1);
	ST7735_DrawCharS(30, 0, 'T', ST7735_Color565(0, 255, 0), 0, 1);
	ST7735_DrawCharS(36, 0, 'E', ST7735_Color565(0, 255, 255), 0, 1);
	ST7735_DrawCharS(42, 0, 'S', ST7735_Color565(0, 0, 255), 0, 1);
	ST7735_DrawCharS(48, 0, 'T', ST7735_Color565(128, 0, 255), 0, 1);

	SysTick_Delay_ms(1000);

	ST7735_DrawCharS(0, 8, 'c', ST7735_Color565(255, 0, 0), 0, 2);
	ST7735_DrawCharS(12, 8, 'h', ST7735_Color565(255, 128, 0), 0, 2);
	ST7735_DrawCharS(24, 8, 'a', ST7735_Color565(255, 255, 0), 0, 2);
	ST7735_DrawCharS(36, 8, 'r', ST7735_Color565(128, 255, 0), 0, 2);
	ST7735_DrawCharS(60, 8, 'T', ST7735_Color565(0, 255, 0), 0, 2);
	ST7735_DrawCharS(72, 8, 'E', ST7735_Color565(0, 255, 255), 0, 2);
	ST7735_DrawCharS(84, 8, 'S', ST7735_Color565(0, 0, 255), 0, 2);
	ST7735_DrawCharS(96, 8, 'T', ST7735_Color565(128, 0, 255), 0, 2);

	SysTick_Delay_ms(1000);

	ST7735_DrawCharS(0, 24, 'c', ST7735_Color565(255, 0, 0), 0, 3);
	ST7735_DrawCharS(18, 24, 'h', ST7735_Color565(255, 128, 0), 0, 3);
	ST7735_DrawCharS(36, 24, 'a', ST7735_Color565(255, 255, 0), 0, 3);
	ST7735_DrawCharS(54, 24, 'r', ST7735_Color565(128, 255, 0), 0, 3);
	ST7735_DrawCharS(90, 24, 'T', ST7735_Color565(0, 255, 0), 0, 3);
	ST7735_DrawCharS(108, 24, 'E', ST7735_Color565(0, 255, 255), 0, 3);
	ST7735_DrawCharS(126, 24, 'S', ST7735_Color565(0, 0, 255), 0, 3);		// Off screen
	ST7735_DrawCharS(144, 24, 'T', ST7735_Color565(128, 0, 255), 0, 3);		// Off screen

	SysTick_Delay_ms(1000);

	ST7735_DrawCharS(0, 48, 'c', ST7735_Color565(255, 0, 0), 0, 4);
	ST7735_DrawCharS(24, 48, 'h', ST7735_Color565(255, 128, 0), 0, 4);
	ST7735_DrawCharS(48, 48, 'a', ST7735_Color565(255, 255, 0), 0, 4);
	ST7735_DrawCharS(72, 48, 'r', ST7735_Color565(128, 255, 0), 0, 4);
	ST7735_DrawCharS(120, 48, 'T', ST7735_Color565(0, 255, 0), 0, 4);		// Off screen
	ST7735_DrawCharS(144, 48, 'E', ST7735_Color565(0, 255, 255), 0, 4);		// Off screen
	ST7735_DrawCharS(168, 48, 'S', ST7735_Color565(0, 0, 255), 0, 4);		// Off screen
	ST7735_DrawCharS(192, 48, 'T', ST7735_Color565(128, 0, 255), 0, 4);		// Off screen

	SysTick_Delay_ms(1000);

	ST7735_DrawCharS(0, 80, 'c', ST7735_Color565(255, 0, 0), 0, 6);
	ST7735_DrawCharS(36, 80, 'h', ST7735_Color565(255, 128, 0), 0, 6);
	ST7735_DrawCharS(72, 80, 'a', ST7735_Color565(255, 255, 0), 0, 6);
	ST7735_DrawCharS(108, 80, 'r', ST7735_Color565(128, 255, 0), 0, 6);		// Off screen

	SysTick_Delay_ms(1000);

	ST7735_DrawChar(0, 135, 'A', ST7735_Color565(255, 128, 128), 0, 2);
	ST7735_DrawChar(12, 135, 'd', ST7735_Color565(255, 0, 0), 0, 2);
	ST7735_DrawChar(24, 135, 'v', ST7735_Color565(255, 128, 0), 0, 2);
	ST7735_DrawChar(48, 135, 'c', ST7735_Color565(255, 255, 0), 0, 2);
	ST7735_DrawChar(60, 135, 'h', ST7735_Color565(128, 255, 0), 0, 2);
	ST7735_DrawChar(72, 135, 'a', ST7735_Color565(0, 255, 0), 0, 2);
	ST7735_DrawChar(84, 135, 'r', ST7735_Color565(0, 255, 255), 0, 2);
	ST7735_DrawChar(108, 135, 't', ST7735_Color565(0, 0, 255), 0, 2);
	ST7735_DrawChar(120, 135, 'e', ST7735_Color565(128, 0, 255), 0, 2);		// Off screen
	ST7735_DrawChar(132, 135, 's', ST7735_Color565(255, 0, 255), 0, 2);		// Off screen
	ST7735_DrawChar(144, 135, 't', ST7735_Color565(255, 255, 255), 0, 2);	// Off screen

	SysTick_Delay_ms(1000);

	ST7735_DrawChar(0, 151, 'A', ST7735_Color565(255, 128, 128), 0, 1);
	ST7735_DrawChar(6, 151, 'd', ST7735_Color565(255, 0, 0), 0, 1);
	ST7735_DrawChar(12, 151, 'v', ST7735_Color565(255, 128, 0), 0, 1);
	ST7735_DrawChar(24, 151, 'c', ST7735_Color565(255, 255, 0), 0, 1);
	ST7735_DrawChar(30, 151, 'h', ST7735_Color565(128, 255, 0), 0, 1);
	ST7735_DrawChar(36, 151, 'a', ST7735_Color565(0, 255, 0), 0, 1);
	ST7735_DrawChar(42, 151, 'r', ST7735_Color565(0, 255, 255), 0, 1);
	ST7735_DrawChar(54, 151, 't', ST7735_Color565(0, 0, 255), 0, 1);
	ST7735_DrawChar(60, 151, 'e', ST7735_Color565(128, 0, 255), 0, 1);
	ST7735_DrawChar(66, 151, 's', ST7735_Color565(255, 0, 255), 0, 1);
	ST7735_DrawChar(72, 151, 't', ST7735_Color565(255, 255, 255), 0, 1);
}

/**
 * Display 'EGR 326' in changing colors
 */
void DrawEGR326_Test(void) {
	uint16_t i, j, r = 100, g = 50, b = 255;

	ST7735_FillScreen(0);	// Set screen to black

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 155; j++) {
			ST7735_DrawChar(4, 68, 'E', ST7735_Color565(r - 50, g, b), 0, 3);
			ST7735_DrawChar(22, 68, 'G', ST7735_Color565(r - 40, g + 10, b - 10), 0, 3);
			ST7735_DrawChar(40, 68, 'R', ST7735_Color565(r - 30, g + 20, b - 20), 0, 3);
			ST7735_DrawChar(74, 68, '3', ST7735_Color565(r - 20, g + 30, b - 30), 0, 3);
			ST7735_DrawChar(92, 68, '2', ST7735_Color565(r - 10, g + 40, b - 40), 0, 3);
			ST7735_DrawChar(110, 68, '6', ST7735_Color565(r, g + 50, b - 50), 0, 3);
			r++;
			g++;
			b--;
		}

		for (j = 155; j > 0; j--) {
			ST7735_DrawChar(4, 68, 'E', ST7735_Color565(r - 50, g, b), 0, 3);
			ST7735_DrawChar(22, 68, 'G', ST7735_Color565(r - 40, g + 10, b - 10), 0, 3);
			ST7735_DrawChar(40, 68, 'R', ST7735_Color565(r - 30, g + 20, b - 20), 0, 3);
			ST7735_DrawChar(74, 68, '3', ST7735_Color565(r - 20, g + 30, b - 30), 0, 3);
			ST7735_DrawChar(92, 68, '2', ST7735_Color565(r - 10, g + 40, b - 40), 0, 3);
			ST7735_DrawChar(110, 68, '6', ST7735_Color565(r, g + 50, b - 50), 0, 3);
			r--;
			g--;
			b++;
		}
	}
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
