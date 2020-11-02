/********************************************
 * Author: Scott VandenToorn
 * Date: September 25, 2020
 * Program: EGR 326-903 Lab 5 - Part 1
 * Description: Uses keypad as input to set
 * 	the current time on the DS3231 RTC IC.
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

/* Global variables */
// States 7 to 1 are for each date value that need to be entered
// State 0 is when the date is written to the RTC
volatile int State = 7;

#define RTC_ADDR 0x68					// Address of DS3231, 1101 000


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	uint8_t key = 0;						// Code of key pressed
	uint8_t key_buffer[2];					// 2 digit key buffer
	uint8_t key_buffer_index = 0;
	unsigned char time_date_payload[8];		// Date sent to RTC
	unsigned char time_date_response[7];	// Date received from RTC

	Init();	// Initializations

	printf("\n\n\nUse keypad to enter a new time & date for the RTC.\n");
	printf("\nEnter year (00-99):\n");

	while(1) {
		if (Keypad_Interrupt) {
			key = Keypad_Read();	// Read keypad

			// Only accept numbers 0 - 9
			if (State > 0 && key > 0 && key != 10 && key != 12) {
				key_buffer[key_buffer_index] = key;	// Store keypress
				key_buffer_index++;

				// Save the two digits entered
				if (key_buffer_index > 1) {
					key_buffer_index = 0;
					State--;

					Keypad_PrintKey(key_buffer[0]);
					Keypad_PrintKey(key_buffer[1]);

					// Turn 11's into 0's
					if (key_buffer[0] == 11) key_buffer[0] = 0;
					if (key_buffer[1] == 11) key_buffer[1] = 0;

					// Convert to binary-coded decimal and save
					time_date_payload[State] = RTC_Integers_to_BCD(key_buffer[0], key_buffer[1]);

					// Prompt for next values
					if (State > 0) {
						switch(State) {
							case 6: printf("\nEnter month (01-12):\n"); break;
							case 5: printf("\nEnter date (01-31):\n"); break;
							case 4: printf("\nEnter day (00-07):\n"); break;
							case 3: printf("\nEnter hours (00-23):\n"); break;
							case 2: printf("\nEnter minutes (00-59):\n"); break;
							case 1: printf("\nEnter seconds (00-59):\n"); break;
						}
					}

					// Send date to RTC
					else {
						printf("\n\n\nSending time & date to DS3231 RTC...\n");

						RTC_I2C1_BurstWrite(RTC_ADDR, 0, 7, time_date_payload);

						printf("\nPayload sent.\n");

						RTC_I2C1_BurstRead(RTC_ADDR, 0, 7, time_date_response);

						printf("\nRTC updated successfully.\n");
						printf("\nPress (*) to display current time & date.\n\n\n");
						RTC_PrintTime(time_date_response);
					}
				}
			}

			// Read date from RTC and print it when (*) pressed
			else if (State == 0 && key == 10) {
				RTC_I2C1_BurstRead(RTC_ADDR, 0, 7, time_date_response);
				RTC_PrintTime(time_date_response);
			}

			Keypad_Enable();	// Start listening for keypresses again
		}

		SysTick_Delay_ms(5);	// Delay while loop
	}
}

/** Initializations */
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
