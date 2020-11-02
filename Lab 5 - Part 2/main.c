/********************************************
 * Author: Scott VandenToorn
 * Date: October 2, 2020
 * Program: EGR 326-903 Lab 5 - Part 2
 * Description: Read and displaying temperature
 * 	on the DS3231 RTC.
 *
 * Pinout:
 *     Keypad rows  -  P2.4 - 2.7
 *  Keypad columns  -  P5.0 - 5.2
 *             SDA  -  P6.4 (SDA)
 *             SCL  -  P6.5 (SCL)
 ********************************************/


/* Includes */
#include <stdio.h>
#include "SysTick.h"
#include "Keypad.h"
#include "DS3231_RTC.h"

/* Function prototypes */
void Init(void);
void Print_Date(unsigned char date[]);
void Print_Time(unsigned char time[]);

/* Global variables */
#define RTC_ADDR 0x68					// Address of DS3231, 1101 000


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	uint8_t key = 0;						// Code of key pressed
	uint8_t key_buffer[2];					// 2 digit key buffer
	uint8_t key_buffer_index = 0;

	Init();	// Initializations

	printf("\n\n*1: Display year, month, and day\n");
	printf("*2: Display hours, minutes, and seconds\n");
	printf("*3: Display temperature (C)\n\n");

	while(1) {
		if (Keypad_Interrupt) {
			key = Keypad_Read();	// Read keypad

			if (key > 0) {
				key_buffer[key_buffer_index] = key;	// Store keypress
				key_buffer_index++;

				// Save the two digits entered
				if (key_buffer_index > 1) {
					key_buffer_index = 0;

					if (key_buffer[0] == 10) {
						// Display year, month, and day
						if (key_buffer[1] == 1) {
							unsigned char date_response[3];
							RTC_I2C1_BurstRead(RTC_ADDR, 0x04, 3, date_response);
							Print_Date(date_response);
						}

						// Display hours, minutes, and seconds
						else if (key_buffer[1] == 2) {
							unsigned char time_response[3];
							RTC_I2C1_BurstRead(RTC_ADDR, 0, 3, time_response);
							Print_Time(time_response);
						}

						// Display temperature (C)
						else if (key_buffer[1] == 3) {
							float temp = RTC_Temperature();
							printf("\nTemperature (C):\t%.2f\n", temp);
						}
					}
				}
			}

			Keypad_Enable();	// Start listening for keypresses again
		}

		SysTick_Delay_ms(5);	// Delay while loop
	}
}


/**
 * Initializations
 */
void Init(void) {
    // Terminate all IO pins
    P1->DIR |= 0xFF; P1->OUT = 0;
    P2->DIR |= 0xFF; P2->OUT = 0;
    P3->DIR |= 0xFF; P3->OUT = 0;
    P4->DIR |= 0xFF; P4->OUT = 0;
    P5->DIR |= 0xFF; P5->OUT = 0;
    P6->DIR |= 0xFF; P6->OUT = 0;
    P7->DIR |= 0xFF; P7->OUT = 0;
    P8->DIR |= 0xFF; P8->OUT = 0;
    P9->DIR |= 0xFF; P9->OUT = 0;
    P10->DIR |= 0xFF; P10->OUT = 0;

	Keypad_Col_P5_Init();	// Port 5 - keypad columns
	Keypad_Row_P2_Init();	// Port 2 - keypad rows
	RTC_I2C1_Init();		// I2C
	SysTick_Init();			// SysTick timer
	__enable_irq();			// Enable interrupts
}

/**
 * Formats and prints date from DS3231 RTC
 * @param date[] Date received from RTC
 */
void Print_Date(unsigned char date[]) {
	int i;
	char months[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// Convert to decimal
	for (i = 0; i < 4; i++) date[i] = RTC_BCD_to_Decimal(date[i]);

	printf("\nDate:\t%s %u, 20%u\n", months[date[1] - 1], date[0], date[2]);
}

/**
 * Formats and prints date from DS3231 RTC
 * @param date[] Date received from RTC
 */
void Print_Time(unsigned char time[]) {
	int i;

	// Convert to decimal
	for (i = 0; i < 4; i++) time[i] = RTC_BCD_to_Decimal(time[i]);

	printf("\nTime:\t%02u:%02u:%02u\n", time[2], time[1], time[0]);
}
