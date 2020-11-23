/*
 * Proximity.h
 *
 *  Created on: Nov 20, 2020
 *      Author: scottvandentoorn
 */

#ifndef PROXIMITY_H_
#define PROXIMITY_H_


#include "msp.h"


void Proximity_GPIO_P4_Init(void);
void Proximity_Trigger(void);


/** Proximity sensor distance (inches) */
volatile uint16_t Backup_Distance;


#endif /* PROXIMITY_H_ */
