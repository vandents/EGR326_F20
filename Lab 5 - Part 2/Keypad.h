/*
 * Keypad.h
 *
 *  Created on: Oct 3, 2020
 *      Author: scottvandentoorn
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_


#include "msp.h"
#include "SysTick.h"


void Keypad_Row_P2_Init(void);
void Keypad_Col_P5_Init(void);
void Keypad_Disable(void);
void Keypad_Enable(void);
uint8_t Keypad_Read(void);
void Keypad_PrintKey(uint8_t key);


volatile uint8_t Keypad_Interrupt;	// 1 after keypress detected, 0 else


#endif /* KEYPAD_H_ */
