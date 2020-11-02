/*
 * DS3231_RTC.h
 *
 *  Created on: Oct 3, 2020
 *      Author: scottvandentoorn
 */

#ifndef DS3231_RTC_H_
#define DS3231_RTC_H_


#include "msp.h"
#include <stdio.h>


void RTC_I2C1_Init(void);
float RTC_Temperature(void);
int RTC_I2C1_BurstWrite(int slave_address, unsigned char mem_address, int byte_count, unsigned char *payload);
int RTC_I2C1_BurstRead(int slave_address, unsigned char mem_address, int byte_count, unsigned char *payload);
void RTC_PrintTime(unsigned char date[]);
unsigned char RTC_BCD_to_Decimal(unsigned char bcd);
unsigned char RTC_Decimal_to_BCD(unsigned char d);
unsigned char RTC_Integers_to_BCD(uint8_t tens, uint8_t ones);


#endif /* DS3231_RTC_H_ */
