/*
 * Stepper.h
 *
 *  Created on: Oct 27, 2020
 *      Author: scottvandentoorn
 */

#ifndef STEPPER_H_
#define STEPPER_H_


#include "msp.h"


void Stepper_Timer32_1_Init(void);
void Stepper_Update(void);
void Stepper_P7_Init(void);


/** Desired stepper position */
volatile int16_t Stepper_Step;
/** True when stepper needs to be updated */
volatile uint8_t Stepper_Interrupt;
/** True when stepper is zeroing itself */
volatile uint8_t Stepper_Zero;


#endif /* STEPPER_H_ */
