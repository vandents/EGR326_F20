/*
 * LCD.c
 *
 *  Created on: Nov 13, 2020
 *      Author: scottvandentoorn
 */


#include "LCD.h"


/**
 * Initialize ST7735 with 'rpm' and 'mph'
 * Pinout:
 *  	SCK  -  P9.5
 *		SDA  -  P9.7
 *		 AO  -  P9.2
 *		RST  -  P9.3
 *		 CS  -  P9.4
 */
void LCD_Init(void) {
	// Character color
	uint16_t white = ST7735_Color565(255, 255, 255);

	ST7735_InitR(INITR_REDTAB);	// Initialize display
	ST7735_SetRotation(3);		// Landscape mode

	// Write 'mph', 'rpm' and decimal places
	ST7735_DrawChar(65, 41, '.', white, 0, 2);
	ST7735_DrawChar(105, 34, 'r', white, 0, 3);
	ST7735_DrawChar(123, 34, 'p', white, 0, 3);
	ST7735_DrawChar(142, 34, 'm', white, 0, 3);
	ST7735_DrawChar(65, 91, '.', white, 0, 2);
	ST7735_DrawChar(105, 84, 'm', white, 0, 3);
	ST7735_DrawChar(123, 84, 'p', white, 0, 3);
	ST7735_DrawChar(142, 84, 'h', white, 0, 3);
}
