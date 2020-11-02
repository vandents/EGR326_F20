#include <stdio.h>
#include "msp.h"
#include "Clock_48MHz.h"

#define S1 BIT1
#define SLAVE_ADDRESS 0x48

char TXData[10] = "RGBBGRBGGR";
int i = 0;

void main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	Clock_Init48MHz();

    P1->DIR &= ~S1;
    P1->REN   = S1;
    P1->OUT   = S1;
    P1->IE       = S1;
    P1->IES     = S1;
    P1->IFG    = 0x00;
    P1->SEL0 |= BIT6 | BIT7; // P1.6 and P1.7 as UCB0SDA and UCB0SCL

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; 		// Hold EUSCI_B0 module in reset state
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MODE_3|EUSCI_B_CTLW0_MST|EUSCI_B_CTLW0_SYNC;
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_UCSSEL_2;		 // Select SMCLK as EUSCI_B0 clock
    EUSCI_B0->BRW = 0x001E;		 // Set BITCLK = BRCLK / (UCBRx+1) = 3 MHz / 30 = 100 kHz
    EUSCI_B0->I2CSA = SLAVE_ADDRESS;
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;		 // Clear SWRST to resume operation

    NVIC->ISER[1] = 0x00000008; 				// Port P1 interrupt is enabled in NVIC
    NVIC->ISER[0] = 0x00100000;				// EUSCI_B0 interrupt is enabled in NVIC
    __enable_irq(); 					// All interrupts are enabled

    while (EUSCI_B0->CTLW0 & EUSCI_B_CTLW0_TXSTP);
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TR | EUSCI_B_CTLW0_TXSTT;
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; 		// Sleep on exit
    __sleep();					 // enter LPM0
}

void EUSCIB0_IRQHandler(void) {
    uint32_t status = EUSCI_B0->IFG; 			// Get EUSCI_B0 interrupt flag
    EUSCI_B0->IFG &=~ EUSCI_B_IFG_TXIFG0; 		// Clear EUSCI_B0 TX inter. flag
    if(status & EUSCI_B_IFG_TXIFG0){ 			// Check if transmit interrupt occurs
        EUSCI_B0->TXBUF = TXData[i++%10]; 	// Load current TXData value to transmit buffer
        EUSCI_B0->IE &= ~EUSCI_B_IE_TXIE0;} 		// Disable EUSCI_B0 TX interrupt
}

void PORT1_IRQHandler(void) {
    uint32_t status = P1->IFG;
    P1->IFG &= ~S1;
    if(status & S1) EUSCI_B0->IE |= EUSCI_B_IE_TXIE0; 	// Enable EUSCI_A0 TX interrupt
}
