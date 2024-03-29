/*
 * Clock_48MHz.h
 *
 *  Created on: Oct 27, 2020
 *      Author: scottvandentoorn
 */

#ifndef CLOCK_48MHZ_H_
#define CLOCK_48MHZ_H_


#include "msp.h"


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
void Clock_Init48MHz(void);



#endif /* CLOCK_48MHZ_H_ */
