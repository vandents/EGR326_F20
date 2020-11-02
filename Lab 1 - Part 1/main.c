/********************************************
 * Author: Scott VandenToorn
 * Date: September 4, 2020
 * Program: EGR 326-10 Lab 1 - Part 1
 * Description: Sequencing colors of a RGB LED
 * using a pushbutton switch
 * Pinout:
 *      P2.6        Button 1
 *      P5.0        Red LED
 *      P5.1        Green LED
 *      P5.2        Blue LED
 ********************************************/

#include "msp.h"

uint8_t debounce(void);
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
    int i, btnPressed = 0;

    // Debounce button
    for (i = 15000; i > 0; i--) {
        if (debounce()) btnPressed = 1;
    }

    // Exit if debounce failed
    if (!btnPressed) return;

    // Determine LED state
    LED_state++;
    if (LED_state > 3) LED_state = 1;
    else if (LED_state < 1) LED_state = 3;

    // Turn on/off LEDs
    switch (LED_state) {
        case 1: red(); break;
        case 2: green(); break;
        case 3: blue(); break;
        default: break;
    }

    // Reset interrupt flag
    P2->IFG &= 0;
}

/* Debounce for P2.6 (button 1) */
uint8_t debounce() {
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

/* Port 5 pin initializations (button) */
void initPort2() {
    // P2.6 to GPIO
    P2->SEL0 &= ~BIT6;
    P2->SEL1 &= ~BIT6;
    // P2.6 to input
    P2->DIR &= ~BIT6;

    // Enable internal resistor
    P2->REN |= BIT6;
    // Enable pull-up resistor (Output high)
    P2->OUT |= BIT6;
    // Set pin interrupt to trigger when it goes from high to low
    P2->IES |= BIT6;
    // Enable interrupt for P2.6
    P2->IE  |= BIT6;
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
