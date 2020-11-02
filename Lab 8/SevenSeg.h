/*
 * SevenSeg.h
 *
 *  Created on: Oct 30, 2020
 *      Author: scottvandentoorn
 * Description: Library for the MAX7219 8 character 7-segment display.
 */

#ifndef SEVENSEG_H_
#define SEVENSEG_H_


#include "msp.h"


void SevenSeg_P2_UCA1_Init(void);
void SevenSeg_MAX7219_Init(void);
void SevenSeg_Write(uint8_t address, uint8_t data);
void SevenSeg_WriteInteger(uint8_t low_digit, uint8_t high_digit, int data);
uint8_t SevenSeg_GetSmallestDigit(uint8_t d);


#endif /* SEVENSEG_H_ */
