/*
 * DS3231_RTC.c
 *
 *  Created on: Oct 3, 2020
 *      Author: scottvandentoorn
 */


#include "DS3231_RTC.h"

#define RTC_ADDR 0x68	// Address of DS3231, 1101 000


/**
 * Configure UCB1 as I2C
 * Program 9-3, 9-4 from Mazidi textbook used as reference
 */
void RTC_I2C1_Init(void) {
	EUSCI_B1->CTLW0 |= 1;		// Disable UCB1
	EUSCI_B1->CTLW0 = 0x0F81;	// Address slave with 7-bit address, master mode, I2C mode,
								// synchronous mode, use SMCLK, receiver, software reset enabled
	EUSCI_B1->BRW = 30;			// Clock pre-scaler, 3 MHz / 30 = 100 kHz
	P6->SEL0 |= 0x30;
	P6->SEL1 &= ~0x30;			// P6.4, 6.5 pin function to UCB1
	EUSCI_B1->CTLW0 &= ~1;		// Enable UCB1
}

/**
 * @return Current temp on RTC
 */
float RTC_Temperature(void) {
	unsigned char temp_response[2];	// Response from RTC
	unsigned char LSB;			// LSB of temp
	signed char MSB;			// MSB of temp

	RTC_I2C1_BurstRead(RTC_ADDR, 0x11, 2, temp_response);

	MSB = temp_response[0];
	LSB = temp_response[1];
	LSB >>= 6;
	LSB &= 0x03;
	return (float) ((float) LSB) * 0.25 + MSB;
}

/**
 * Write multiple bytes to DS3231 RTC slave over I2C using UCB1.
 * Program 9-3 from Mazidi textbook used as reference.
 *
 * @param slave_address Address of slave
 * @param mem_address Slave memory address pointer
 * @param byte_count Number of bytes in data
 * @param *payload Pointer to start BCD array date
 * @return 0 if succes, -1 if failure
 */
int RTC_I2C1_BurstWrite(int slave_address, unsigned char mem_address, int byte_count, unsigned char *payload) {
	if (byte_count <= 0) return -1;

	EUSCI_B1->I2CSA = slave_address;	// Write slave address to I2CSA register
	EUSCI_B1->CTLW0 |= 0x0010;			// Enable transmitter
	EUSCI_B1->CTLW0 |= 0x0002;			// Send START condition and send slave address
	while(!(EUSCI_B1->IFG & 2)); 		// Wait for transmitter

	EUSCI_B1->TXBUF = mem_address;		// Send register pointer
	while(!(EUSCI_B1->IFG & 2)); 		// Wait for transmitter

	// Send one byte at a time
	while(byte_count) {
		EUSCI_B1->TXBUF = *payload++;	// Send byte, increment pointer position
		while(!(EUSCI_B1->IFG & 2)); 	// Wait for transmitter
		byte_count--;
	}

	EUSCI_B1->CTLW0 |= 0x0004;			// Send STOP condition
	while(EUSCI_B1->CTLW0 & 4);			// Wait for STOP to be sent

	RTC_I2C1_Init();

	return 0;
}

/**
 * Read multiple bytes from DS3231 RTC slave over I2C using UCB1.
 * Program 9-4 from Mazidi textbook used as reference
 *
 * @param slave_address Address of slave
 * @param mem_address Slave memory address pointer
 * @param byte_count Number of bytes in data
 * @param *payload Array where date will be saved
 * @return 0 if succes, -1 if failure
 */
int RTC_I2C1_BurstRead(int slave_address, unsigned char mem_address, int byte_count, unsigned char *payload) {
	uint16_t i;

	if (byte_count <= 0) return -1;

	EUSCI_B1->I2CSA = slave_address;	// Write slave address to I2CSA register
	EUSCI_B1->CTLW0 |= 0x0010;			// Enable transmitter
	EUSCI_B1->CTLW0 |= 0x0002;			// Send START condition and send slave address
	while(!(EUSCI_B1->IFG & 2)); 		// Wait for transmitter

	EUSCI_B1->TXBUF = mem_address;		// Send register pointer
	while(!(EUSCI_B1->IFG & 2)); 		// Wait for transmitter

	EUSCI_B1->CTLW0 &= ~0x0010;			// Enable receiver
	EUSCI_B1->CTLW0 |= 0x0002;			// Send RESTART condition and send slave address
	while(EUSCI_B1->CTLW0 & 2);			// Wait for restart to be sent
	while(!(EUSCI_B1->IFG & 1)); 		// Wait for receiver

	// Read one byte at a time
	for (i = 0; i < byte_count; i++) {
		// Configure to send STOP condition after last byte is received
		if (i == byte_count - 1) EUSCI_B1->CTLW0 |= 0x0004;

		payload[i] = EUSCI_B1->RXBUF;	// Read and save byte
		while(!(EUSCI_B1->IFG & 1));	// Wait for receiver
	}

	while(EUSCI_B1->CTLW0 & 4);			// Wait for STOP to be sent

	RTC_I2C1_Init();

	return 0;
}

/**
 * Formats and prints date from DS3231 RTC
 * @param date[] Date received from RTC
 */
void RTC_PrintTime(unsigned char date[]) {
	int i;
	char days[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char months[][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// Convert to decimal
	for (i = 0; i < 7; i++) date[i] = RTC_BCD_to_Decimal(date[i]);

	printf("Current time & date:\t");
	printf("%s %s %u ", days[date[3] - 1], months[date[5] - 1], date[4]);
	printf(" %02u:%02u:%02u\n\n", date[2], date[1], date[0]);
}

/**
 * @param bcd A binary-coded decimal
 * @return Corresponding decimal value
 */
unsigned char RTC_BCD_to_Decimal(unsigned char bcd) {
	return (bcd & 0x0F) + (((bcd & 0xF0) >> 4) * 10);
}

/**
 * @param d A Decimal number
 * @return Corresponding binary-coded decimal
 */
unsigned char RTC_Decimal_to_BCD(unsigned char d) {
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}

/**
 * @param tens 10s place digit
 * @param tens 1s place digit
 * @return Corresponding 2 digit number represented in a binary-coded decimal
 */
unsigned char RTC_Integers_to_BCD(uint8_t tens, uint8_t ones) {
	unsigned char i = 10 * tens + ones;
	return (((i / 10) << 4) & 0xF0) | ((i % 10) & 0x0F);
}
