/********************************************
 * Author: Scott VandenToorn
 * Date: October 9, 2020
 * Program: EGR 326-903 Lab 6 - Part 2
 * Description: Display bitmap splash and then
 * 	display fake menu.
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
#include "pepe.h"

/* Function prototypes */
void Clock_Init48MHz(void);
void SplashScreen(void);
void menu(void);
void draw_option(int y, char *text, int length);


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;		// Stop watchdog timer

	Clock_Init48MHz();
	SysTick_Init();
	ST7735_InitR(INITR_REDTAB);

	SplashScreen();				// Show splash screen
	menu();						// Show menu

	while(1) { }
}


/**
 * Display splash screen bitmap
 */
void SplashScreen(void) {
	int i, j;
	// Draw bitmap
	ST7735_DrawBitmap(0, 160, pepe, 128, 160);

	// "Loading" text at bottom
	ST7735_DrawChar(1,  152, 'L', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(7,  152, 'o', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(13, 152, 'a', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(19, 152, 'd', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(25, 152, 'i', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(31, 152, 'n', ST7735_Color565(255, 255, 255), 0, 1);
	ST7735_DrawChar(37, 152, 'g', ST7735_Color565(255, 255, 255), 0, 1);

	// Ellipsis animation
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 4; j++) {
			ST7735_DrawChar(43 + j * 6, 152, '.', ST7735_Color565(0, 0, 0), 0, 1);
		}
		for (j = 0; j < 4; j++) {
			SysTick_Delay_ms(250);
			ST7735_DrawChar(43 + j * 6, 152, '.', ST7735_Color565(255, 255, 255), 0, 1);
		}
		SysTick_Delay_ms(500);
	}
}

/**
 * Draw a fake menu
 */
void menu(void) {
	ST7735_FillScreen(0);	// Set screen to black
	draw_option(92,  "Audio", 5);
	draw_option(114, "Destruct", 8);
	draw_option(68,  "Temp.", 5);
	draw_option(44,  "Time/Date", 9);
}

/**
 * Draw a menu option
 * @param y Vertical position
 * @param *text Menu text string
 * @param Length of menu text
 */
void draw_option(int y, char *text, int length) {
	int i;
	for (i = 0; i < length; i++) {
		ST7735_DrawChar(8 + i * 12, y, text[i], ST7735_Color565(255, 255, 255), 0, 2);
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
