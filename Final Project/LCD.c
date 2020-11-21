/*
 * LCD.c
 *
 *  Created on: Nov 13, 2020
 *      Author: scottvandentoorn
 */

#include "LCD.h"


const char Days[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
volatile uint8_t Scroll_Text[] = {
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '.', ' ',
	' ', ' ', ':', ' ', ' ', ' ', ' ', ' ', ' ', 'O', 'F', ' '
};
uint16_t White, Blue;	// Background and character color


/**
 * Initialize LCD displays A and B, ADC and PWM for brightness
 * Pinout:
 *  	SCK  -  P9.5
 *		SDA  -  P9.7
 *		 AO  -  P9.2
 *		RST  -  P9.3
 *	   CS A  -  P9.0
 *	   CS B  -  P9.1
 */
void LCD_Init(void) {
	// Configure GPIO for LCD_A and LCD_B CS pin (P9.0 - 9.1)
	P9->SEL0 &= ~0x03;
	P9->SEL1 &= ~0x03;					// GPIO
	P9->DIR |= 0x03;					// Output
	P9->OUT |= 0x03;					// Output HIGH

	White = ST7735_Color565(255, 255, 255);
	Blue = ST7735_Color565(0, 75, 195);

	ST7735_InitR(INITR_REDTAB, 0x03);	// Initialize displays
	ST7735_SetRotation(3, 0x03);		// Landscape mode
	ST7735_FillScreen(White, 0x03);

	LCD_B_Init();
	LCD_A_State = 0x00;

	/* PWM for brightness, more TimerA2 config in Motor.c */
	P5->SEL0 |= 0x40;
	P5->SEL1 &= ~0x40;					// P5.6 to TimerA2.1
	P5->DIR |= 0x40;					// Output
	TIMER_A2->CCR[1] = 1800;			// Duty cycle
	TIMER_A2->CCTL[1] = 0xE0;			// Reset/set mode

	/* Photoresistor ADC, more ADC config in Motor.c */
	P6->SEL0 |= 0x02;
	P6->SEL1 |= 0x02;					// P6.1 to ADC14 pin function
	ADC14->MCTL[4] = 0x0E;				// A4 to input channel
	ADC14->IER0 |= 0x10;				// Enable A4 interrupt
}

void LCD_A_Scroll(void) {
	static uint16_t index = 0;
	int i, j = 0;

	for (i = index; i < index + 10; i++) {
		if (i > 24) break;

		if (Scroll_Text[i] == 'O') {
			ST7735_DrawChar(1 + j * 17, 55, ' ', Blue, White, 3, LCD_A_CS);
			ST7735_DrawChar(9 + j * 17, 55, Scroll_Text[i], Blue, White, 1, LCD_A_CS);
		} else if (Scroll_Text[i] == '.') {
			ST7735_DrawChar(1 + j * 17, 55, ' ', Blue, White, 3, LCD_A_CS);
			ST7735_DrawChar(0 + j * 17, 62, Scroll_Text[i], Blue, White, 2, LCD_A_CS);
		} else ST7735_DrawChar(1 + j * 17, 55, Scroll_Text[i], Blue, White, 3, LCD_A_CS);
		j++;
	}
	index = index > 24 ? 0 : index + 1;
}

void LCD_A_Scroll_SetText(unsigned char response[3], float rtc_temp) {
	int int_rtc_temp = (int) rtc_temp;
	const char *day = Days[(response[2] & 0x07) - 1];
	uint8_t minutes_10 = ((response[0] & 0x70) >> 4) + '0',
		minutes_1 = (response[0] & 0x0F) + '0',
		hours_10 = ((response[1] & 0x10) >> 4) + '0',
		hours_1 = (response[1] & 0x0F) + '0',
		temp_1 = (int_rtc_temp % 10) + '0',
		temp_10 = int_rtc_temp > 10 ? ((int_rtc_temp / 10) % 10) + '0' : ' ',
		temp_100 = int_rtc_temp > 100 ? (int_rtc_temp / 100) + '0' : ' ';

	Scroll_Text[8] = day[0];
	Scroll_Text[9] = day[1];
	Scroll_Text[10] = day[2];
	Scroll_Text[13] = hours_10;
	Scroll_Text[14] = hours_1;
	Scroll_Text[16] = minutes_10;
	Scroll_Text[17] = minutes_1;
	Scroll_Text[19] = temp_100;
	Scroll_Text[20] = temp_10;
	Scroll_Text[21] = temp_1;
}

void LCD_A_Menu_Show(void) {
	ST7735_FillScreen(White, LCD_A_CS);

	// Menu
	ST7735_DrawChar(3, 3, 'M', Blue, White, 4, LCD_A_CS);
	ST7735_DrawChar(27, 3, 'e', Blue, White, 4, LCD_A_CS);
	ST7735_DrawChar(51, 3, 'n', Blue, White, 4, LCD_A_CS);
	ST7735_DrawChar(75, 3, 'u', Blue, White, 4, LCD_A_CS);

	// Alarm Log
	ST7735_DrawChar(35, 50, 'A', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(47, 50, 'l', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(59, 50, 'a', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(71, 50, 'r', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(83, 50, 'm', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(95, 50, ' ', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(107, 50, 'L', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(119, 50, 'o', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(131, 50, 'g', Blue, White, 2, LCD_A_CS);

	// Set Time
	ST7735_DrawChar(35, 75, 'S', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(47, 75, 'e', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(59, 75, 't', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(71, 75, ' ', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(83, 75, 'T', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(95, 75, 'i', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(107, 75, 'm', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(119, 75, 'e', Blue, White, 2, LCD_A_CS);

	// Exit
	ST7735_DrawChar(35, 100, 'E', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(47, 100, 'x', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(59, 100, 'i', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(71, 100, 't', Blue, White, 2, LCD_A_CS);
}

void LCD_A_Menu_MoveCursor(uint8_t option) {
	ST7735_DrawChar(15, 50, option == 0 ? 0x10 : ' ', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(15, 75, option == 1 ? 0x10 : ' ', Blue, White, 2, LCD_A_CS);
	ST7735_DrawChar(15, 100, option == 2 ? 0x10 : ' ', Blue, White, 2, LCD_A_CS);
}

/**
 * Initialize LCD B with 'rpm' and 'mph'
 */
void LCD_B_Init(void) {
	// Write 'mph', 'rpm' and decimal places
	ST7735_DrawChar(65, 41, '.', Blue, White, 2, LCD_B_CS);
	ST7735_DrawChar(105, 34, 'r', Blue, White, 3, LCD_B_CS);
	ST7735_DrawChar(123, 34, 'p', Blue, White, 3, LCD_B_CS);
	ST7735_DrawChar(142, 34, 'm', Blue, White, 3, LCD_B_CS);
	ST7735_DrawChar(65, 91, '.', Blue, White, 2, LCD_B_CS);
	ST7735_DrawChar(105, 84, 'm', Blue, White, 3, LCD_B_CS);
	ST7735_DrawChar(123, 84, 'p', Blue, White, 3, LCD_B_CS);
	ST7735_DrawChar(142, 84, 'h', Blue, White, 3, LCD_B_CS);
}

/**
 * Write current speed to LCD display
 */
void LCD_B_WriteSpeedRPM(float speed, float rpm) {
	// Integer portion of RPM and Speed
	uint16_t rpm_int = (uint16_t) rpm, speed_int = (uint16_t) speed;

	// Character for each digit
	char rpm_100 = rpm_int > 99 ? rpm_int / 100 + '0' : ' ',
		rpm_10 = rpm_int > 9 ? (rpm_int / 10) % 10 + '0' : ' ',
		rpm_1 = rpm_int % 10 + '0',
		rpm_dec = (rpm - rpm_int) * 10 + '0',
		speed_100 = speed_int > 99 ? speed_int / 100 + '0' : ' ',
		speed_10 = speed_int > 9 ? (speed_int / 10) % 10 + '0' : ' ',
		speed_1 = speed_int % 10 + '0',
		speed_dec = (speed - speed_int) * 10 + '0';

	// Character of each digit last time
	static char rpm_100_last, rpm_10_last, rpm_1_last, rpm_dec_last,
		speed_100_last, speed_10_last, speed_1_last, speed_dec_last;

	// Write RPM of digits that changed
	if (rpm_100 != rpm_100_last) ST7735_DrawChar(1, 27, rpm_100, Blue, White, 4, LCD_B_CS);
	if (rpm_10 != rpm_10_last) ST7735_DrawChar(23, 27, rpm_10, Blue, White, 4, LCD_B_CS);
	if (rpm_1 != rpm_1_last) ST7735_DrawChar(45, 27, rpm_1, Blue, White, 4, LCD_B_CS);
	if (rpm_dec != rpm_dec_last) ST7735_DrawChar(76, 27, rpm_dec, Blue, White, 4, LCD_B_CS);

	// Write Speed of digits that changed
	if (speed_100 != speed_100_last) ST7735_DrawChar(1, 77, speed_100, Blue, White, 4, LCD_B_CS);
	if (speed_10 != speed_10_last) ST7735_DrawChar(23, 77, speed_10, Blue, White, 4, LCD_B_CS);
	if (speed_1 != speed_1_last) ST7735_DrawChar(45, 77, speed_1, Blue, White, 4, LCD_B_CS);
	if (speed_dec != speed_dec_last) ST7735_DrawChar(76, 77, speed_dec, Blue, White, 4, LCD_B_CS);

	// Record characters
	rpm_100_last = rpm_100;
	rpm_10_last = rpm_10;
	rpm_1_last = rpm_1;
	rpm_dec_last = rpm_dec;
	speed_100_last = speed_100;
	speed_10_last = speed_10;
	speed_1_last = speed_1;
	speed_dec_last = speed_dec;
}
