/*
 * Rotary.h
 *
 *  Created on: Nov 18, 2020
 *      Author: scottvandentoorn
 */

#ifndef ROTARY_H_
#define ROTARY_H_


#include "msp.h"


void Rotary_P3_GPIO_Init(void);


/** Flag set when button pressed */
volatile uint8_t Rotary_Button;
/** Flag set when encoder turned CW */
volatile uint8_t Rotary_CW;
/** Flag set when encoder turned CCW */
volatile uint8_t Rotary_CCW;


#endif /* ROTARY_H_ */
