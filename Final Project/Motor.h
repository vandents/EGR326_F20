/*
 * Motor.h
 *
 *  Created on: Nov 19, 2020
 *      Author: scottvandentoorn
 */

#ifndef MOTOR_H_
#define MOTOR_H_


#include "msp.h"


void Motor_Potentiometer_P6_A5_Init(void);
void Motor_P5_TA2_2_Init(void);
void Motor_HallEffect_P5_Init(void);
void Motor_PWM_ADC_Init(void);


/** Set when hall sensors trigger */
volatile uint8_t Motor_Spinning;
/** Motor RPM */
volatile float RPM;
/** Motor speed */
volatile float Speed;


#endif /* MOTOR_H_ */
