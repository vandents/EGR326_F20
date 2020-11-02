/********************************************
 * Author: Scott VandenToorn
 * Date: September 4, 2020
 * Program: EGR 326-10 Lab 1 - Part 2
 * Description: Sequencing colors of a RGB LED
 * using two switches with reverse direction
 * Pinout:
 *      P2.6        Button 1
 *      P2.7        Button 2
 *      P5.0        Red LED
 *      P5.1        Green LED
 *      P5.2        Blue LED
 ********************************************/

#include "msp.h"

uint8_t debounce1(void);
uint8_t debounce2(void);
void off(void);
void red(void);
void green(void);
void blue(void);
void initPort2(void);
void initPort5(void);

volatile int LED_state = 0;

void main() {
    // Stop watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    initPort2();
	initPort5();

	while(1) { }
}

/* Port 2 ISR */
void PORT2_IRQHandler() {
    int i, btn1Pressed = 0, btn2Pressed = 0;

    // Debounce buttons
    for (i = 15000; i > 0; i--) {
        if (debounce1()) btn1Pressed = 1;
        if (debounce2()) btn2Pressed = 1;
    }

    // Determine LED state
    if (btn1Pressed && btn2Pressed) LED_state = 0;
    else {
        if (btn1Pressed) LED_state++;
        else if (btn2Pressed) LED_state--;

        if (LED_state > 3) LED_state = 1;
        else if (LED_state < 1) LED_state = 3;
    }

    // Turn on/off LEDs
    switch (LED_state) {
        case 0: off(); break;
        case 1: red(); break;
        case 2: green(); break;
        case 3: blue(); break;
        default: break;
    }

    // Reset interrupt flag
    P2->IFG &= 0;
}

/* Debounce for P2.6 (button 1) */
uint8_t debounce1() {
    // Current debounce status
    static uint16_t State = 0;
    // Read switch, upper 5 bits of State are don't cares
    State = (State << 1) | (P2IN & BIT6) >> 1 | 0xf800;
    // Indicates 0 level is stable for 10 consecutive calls
    if (State == 0xfc00) {
        State = 0;
        return 1;
    }
    return 0;
}

/* Debounce for P2.7 (button 2) */
uint8_t debounce2() {
    // Current debounce status
    static uint16_t State = 0;
    // Read switch, upper 5 bits of State are don't cares
    State = (State << 1) | (P2IN & BIT7) >> 1 | 0xf800;
    // Indicates 0 level is stable for 10 consecutive calls
    if (State == 0xfc00) {
        State = 0;
        return 1;
    }
    return 0;
}

void off() {
    P5->OUT &= ~(BIT0 | BIT1 | BIT2);
}

void red() {
    off();
    P5->OUT ^= BIT0;
}

void green() {
    off();
    P5->OUT ^= BIT1;
}

void blue() {
    off();
    P5->OUT ^= BIT2;
}

/* Port 5 pin initializations (buttons) */
void initPort2() {
    // P2.6 - 2.7 to GPIO
    P2->SEL0 &= ~(BIT6 | BIT7);
    P2->SEL1 &= ~(BIT6 | BIT7);
    // P2.6 - 2.7 to input
    P2->DIR &= ~(BIT6 | BIT7);

    // Enable internal resistor
    P2->REN |= (BIT6 | BIT7);
    // Enable pull-up resistor (Output high)
    P2->OUT |= (BIT6 | BIT7);
    // Set pin interrupt to trigger when it goes from high to low
    P2->IES |= (BIT6 | BIT7);
    // Enable interrupt for P2.6 - 2.7
    P2->IE  |= (BIT6 | BIT7);
    // Reset interrupt flag
    P2->IFG = 0;

    // Enable Port 2 interrupt on the NVIC
    NVIC_EnableIRQ(PORT2_IRQn);
}

/* Port 5 pin initializations (LEDs) */
void initPort5() {
    // P5.0 - 5.2 to GPIO
    P5->SEL0 &= ~(BIT0 | BIT1 | BIT2);
    P5->SEL1 &= ~(BIT0 | BIT1 | BIT2);
    // P5.0 - 5.2 to output
    P5->DIR |= (BIT0 | BIT1 | BIT2);
    off();
}
