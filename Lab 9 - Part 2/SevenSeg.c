/*
 * SevenSeg.c
 *
 *  Created on: Oct 30, 2020
 *      Author: scottvandentoorn
 * Description: Library for the MAX7219 8 character 7-segment display.
 */

#include "SevenSeg.h"


/**
 * Configure UCA1 for SPI
 * 	P2.3 - CLK
 * 	P2.4 - SIMO
 * 	P2.5 - CS (GPIO)
 */
void SevenSeg_P2_UCA1_Init(void) {
	// UCA1 to SPI
    EUSCI_A1->CTLW0 = 0x0001;			// Disable during config
	EUSCI_A1->CTLW0 = 0xA981;			// MSB, 8-bit, master, 3-pin, synch, SMCLK
	EUSCI_A1->BRW = 2;					// 12MHz / 2 = 6 MHz clock
	EUSCI_A1->CTLW0 &= ~0x0001;			// Enable

	// CLK and DIN (P2.3 - 2.4)
    PMAP->KEYID = 0x2D52;				// Unlock PMAP
    P2MAP->PMAP_REGISTER[1] = 0x0800;	// Map P2.3 to PM_UCA1CLK
    P2MAP->PMAP_REGISTER[2] = 0x070A;	// Map P2.4 to PM_UCA1SIMO
	P2->SEL0 |= 0x18;
	P2->SEL1 &= ~0x18;					// P2.3 - 2.4 pin function to UCB3
    PMAP->CTL = 0;
    PMAP->KEYID = 0;					// Lock PMAP

    // CS (P2.5)
	P2->SEL0 &= ~0x20;
	P2->SEL1 &= ~0x20;					// P2.5 to GPIO
    P2->DIR |= 0x20;					// P2.5 to Output
    P2->OUT |= 0x20;					// P2.5 to Output HIGH

    SevenSeg_MAX7219_Init();			// Init MAX7219 display driver
}

/**
 * Configure MAX7219 display driver and init digits
 */
void SevenSeg_MAX7219_Init(void) {
	int i;
	uint8_t config[] = {
		0x09, 0x0F,					// Decode Mode: Decode digits 0 - 3
		0x0A, 0x07,					// Intensity: 50%
		0x0B, 0x07,					// Scan Limit: Display all digits
		0x0C, 0x01,					// Shutdown: Disable shutdown
		0x0F, 0x00,					// Test: Disable test mode
		0x01, 0x0F,					// Off
		0x02, 0x0F,					// Off
		0x03, 0x0F,					// Off
		0x04, 0x0F,					// Off
		0x05, 0xBD,					// 'd.', 10111101
		0x06, 0x4F,					// 'E',  01001111
		0x07, 0x67,					// 'P',  01100111
		0x08, 0x5B,					// 'S',  01011011
	};

	for (i = 0; i < 25; i++)			// Send configuration
		SevenSeg_Write(config[i], config[++i]);
}

/**
 * Send 16-bits of data to the MAX7219 driver
 * @param address Address of register to write to
 * @param data Data to be written
 */
void SevenSeg_Write(uint8_t address, uint8_t data) {
	P2->OUT &= ~0x20;				// CS LOW

	while(!(EUSCI_A1->IFG & 0x02));	// Wait for tx buffer to clear
	EUSCI_A1->TXBUF = address;		// Send address (upper 8 bits)

	while(EUSCI_A1->STATW & 0x01);	// Wait for transmission to complete
	EUSCI_A1->TXBUF = data;			// Send data (lower 8 bits)

	while(EUSCI_A1->STATW & 0x01);	// Wait for transmission to complete
	P2->OUT |= 0x20;				// CS HIGH
}

/**
 * Write an integer to a given range of digits
 * @param low_digit Low digit (inclusive, 0 - 7)
 * @param high_digit High digit (inclusive, 0 - 7)
 * @param data Integer to be written to MAX7219
 */
void SevenSeg_WriteInteger(uint8_t low_digit, uint8_t high_digit, int data) {
	uint8_t i;
	for (i = low_digit; i <= high_digit; i++) {
		SevenSeg_Write(i + 1, data % 10);	// Send smallest digit
		data /= 10;							// Remove smallest digit
	}
}

/**
 * Write a float (to one decimal place) to a given range of digits
 * @param low_digit Low digit (inclusive, 0 - 7)
 * @param high_digit High digit (inclusive, 0 - 7)
 * @param data Float to be written to MAX7219
 */
void SevenSeg_WriteFloat(uint8_t low_digit, uint8_t high_digit, float data) {
	int int_data = (int) data;
	uint8_t decimal = (uint8_t) ((data - int_data) * 10);

	// Write decimal portion
	SevenSeg_Write(low_digit + 1, decimal);
	// Write 1's place w/ dp
	SevenSeg_Write(low_digit + 2, (int_data % 10) | 0x80);
	// Write 10's place
	SevenSeg_Write(low_digit + 3, int_data > 10 ? (int_data / 10) % 10 : 0x0F);
	// Write 100's place
	SevenSeg_Write(low_digit + 4, int_data > 100 ? int_data / 100 : 0x0F);
}
