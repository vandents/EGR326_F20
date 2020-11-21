/*
 * LCD.h
 *
 *  Created on: Nov 13, 2020
 *      Author: scottvandentoorn
 */

#ifndef LCD_H_
#define LCD_H_


#include "msp.h"
#include "ST7735.h"


#define LCD_A_CS 0x01
#define LCD_B_CS 0x02


void LCD_Init(void);
void LCD_B_Init(void);
void LCD_A_Scroll(void);
void LCD_A_Scroll_SetText(unsigned char response[3], float rtc_temp);
void LCD_A_Menu_Show(void);
void LCD_A_Menu_MoveCursor(uint8_t option);
void LCD_B_WriteSpeedRPM(float speed, float rpm);


uint8_t LCD_A_State;


#endif /* LCD_H_ */
