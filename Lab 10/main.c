/********************************************
 * Author: Scott VandenToorn
 * Date: October 30, 2020
 * Program: EGR 326-903 Lab 10
 * Description: Watchdog timer demo.
 *
 * Pinout:
 *  	SCK  -  P9.5
 *		SDA  -  P9.7
 *		 AO  -  P9.2
 *		RST  -  P9.3
 *		 CS  -  P9.4
 *	 Button  -  P3.6
 *	 	LED  -  P2.2
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "ST7735.h"
#include "Time.h"

/* Function prototypes */
void Init(void);
void Button_P3_Init(void);
void LED_P2_Init(void);

/* Global variables */
volatile uint16_t Second_Count;


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

	while(1) { }
}


/********************************************
 * Initializations
 ********************************************/

/**
 * Initialize clocks, timers, ADC and GPIO
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

    Clock_Frequencies_Init();
	SYSCTL->WDTRESET_CTL |= 0x00;	// WDT Soft reset
	WDT_A->CTL = 0x5A2C;			// Start WDT, 4s timeout
	SysTick_Init();
	Button_P3_Init();
	LED_P2_Init();
	Timer32_1_Interrupt_Init(10);	// 1 Hz interrupt
	ST7735_InitR(INITR_REDTAB);		// Initialize display
	ST7735_SetRotation(3);			// Landscape mode
	__enable_irq();					// Enable interrupts

	Second_Count = 0;				// Reset second count

	// Init text on LCD
	uint16_t white = ST7735_Color565(255, 128, 128);	// Character color
	ST7735_DrawChar(16, 50, 'S', white, 0, 4);
	ST7735_DrawChar(38, 50, 'E', white, 0, 4);
	ST7735_DrawChar(60, 50, 'C', white, 0, 4);
	ST7735_DrawChar(82, 50, ':', white, 0, 4);
	ST7735_DrawChar(104, 50, ' ', white, 0, 4);
	ST7735_DrawChar(126, 50, Second_Count + '0', white, 0, 4);
}

/**
 * Init button on P3.6
 */
void Button_P3_Init(void) {
    P3->SEL0 &= ~0x40;
    P3->SEL1 &= ~0x40;	// GPIO
    P3->DIR &= ~0x40;	// Input
    P3->REN |= 0x40;	// Enable pull resistors
    P3->OUT |= 0x40;	// Pull up selected
    P3->IE |= 0x40;		// Enable interrupts
    P3->IES |= 0x40;	// Trigger on falling edge
    P3->IFG = 0;		// Reset interrupt flag
    NVIC_EnableIRQ(PORT3_IRQn);	// Enable interrupts on NVIC
}

/**
 * Init LED on P2.2
 */
void LED_P2_Init(void) {
	P2->SEL0 &= ~BIT2;
    P2->SEL1 &= ~BIT2;	// P2.2 to GPIO
    P2->DIR |= BIT2;	// Output
    P2->OUT |= BIT2;	// LED on
}


/********************************************
 * Interrupts
 ********************************************/

/**
 * Port 3 ISR (button)
 */
void PORT3_IRQHandler(void) {
	if (P3->IFG & 0x40) {
		Second_Count = 0;
		WDT_A->CTL = 0x5A2C;	// Reset WDT
		// Update LCD
		ST7735_DrawChar(126, 50, Second_Count + '0', ST7735_Color565(255, 128, 128), 0, 4);
	}
	P3->IFG = 0;
}

/**
 * 1 Hz interrupt
 */
void T32_INT1_IRQHandler(void) {
	TIMER32_1->INTCLR = 0;	// Reset interrupt flag
    P2->OUT &= ~BIT2;		// LED off
	Second_Count++;
	ST7735_DrawChar(126, 50, Second_Count + '0', ST7735_Color565(255, 128, 128), 0, 4);
}
