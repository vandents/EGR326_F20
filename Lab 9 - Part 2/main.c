/********************************************
 * Author: Scott VandenToorn
 * Date: October 23, 2020
 * Program: EGR 326-903 Lab 9 - Part 2
 * Description: Determine the rate of rotation of a
 * 	stepper motor. Mount one or two magnets to the
 * 	modified stepper motor shaft (without gears)
 * 	used in lab 6. The student will align their hall
 * 	effect sensor close to the rotating magnet(s)
 * 	and determine the the rate of rotation (RPM)
 * 	which will be displayed on the LCD.
 *
 * Pinout:
 * 			   Motor PWM  -  P5.7
 * 	Hall effect switches  -  P5.4 - 5.5
 * 	   10k potentiometer  -  P6.0
 *  				 SCK  -  P9.5
 *					 SDA  -  P9.7
 *					  AO  -  P9.2
 *					 RST  -  P9.3
 *					  CS  -  P9.4
 ********************************************/


/* Includes */
#include <stdio.h>
#include "msp.h"
#include "ST7735.h"
#include "LCD.h"
#include "Time.h"
#include "SevenSeg.h"

/* Function prototypes */
void Init(void);
void Motor_P5_TA2_2_Init(void);
void Potentiometer_P6_A5_Init(void);
void HallEffect_P5_Init(void);
void ST7735_WriteSpeedRPM(void);

/* Global variables */
volatile uint8_t Update_Gauges = 0;
volatile uint8_t Motor_Spinning = 0;
volatile float Speed = 0;
volatile float RPM = 0;
const uint16_t ADC_PWM_Vector[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 800, 800, 800,
	800, 800, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813,
	814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 829, 831,
	833, 835, 837, 839, 841, 843, 845, 847, 849, 851, 853, 855, 857, 859, 861, 863,
	865, 867, 869, 871, 873, 875, 877, 879, 881, 884, 887, 890, 893, 896, 899, 902,
	905, 908, 911, 914, 917, 920, 923, 926, 929, 932, 935, 938, 941, 944, 947, 950,
	953, 956, 959, 962, 966, 970, 974, 978, 982, 986, 990, 994, 998, 1002, 1006,
	1010, 1014, 1018, 1022, 1026, 1030, 1034, 1038, 1042, 1046, 1050, 1054, 1058,
	1062, 1066, 1070, 1075, 1080, 1085, 1090, 1095, 1100, 1105, 1110, 1115, 1120,
	1125, 1130, 1135, 1140, 1145, 1150, 1155, 1160, 1165, 1170, 1175, 1180, 1185,
	1190, 1195, 1200, 1205, 1211, 1217, 1223, 1229, 1235, 1241, 1247, 1253, 1259,
	1265, 1271, 1277, 1283, 1289, 1295, 1301, 1307, 1313, 1319, 1325, 1331, 1337,
	1343, 1349, 1355, 1361, 1367, 1374, 1381, 1388, 1395, 1402, 1409, 1416, 1423,
	1430, 1437, 1444, 1451, 1458, 1465, 1472, 1479, 1486, 1493, 1500, 1507, 1514,
	1521, 1528, 1535, 1542, 1549, 1556, 1564, 1572, 1580, 1588, 1596, 1604, 1612,
	1620, 1628, 1636, 1644, 1652, 1660, 1668, 1676, 1684, 1692, 1700, 1708, 1716,
	1724, 1732, 1740, 1748, 1756, 1764, 1772, 1781, 1790, 1799, 1800, 1800, 1800,
	1800, 1800, 1800, 1800, 1800, 1800, 1800
};


/**
 * main.c
 */
void main(void) {
	WDT_A->CTL = 0x5A80;	// Stop watchdog timer

	Init();

	while (1) {
		if (Update_Gauges) {
			SevenSeg_WriteFloat(0, 3, Speed);	// Update 7-segment
			ST7735_WriteSpeedRPM();				// Update LCD
			Update_Gauges = 0;					// Reset flag
		}
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

	Clock_48MHz_Init();
	SysTick_Init();
	Motor_P5_TA2_2_Init();
	Potentiometer_P6_A5_Init();
	SevenSeg_P2_UCA1_Init();
	Timer32_1_Interrupt_Init(2);	// 0.2 Hz interrupt
	Timer32_2_Counter_Init();
	HallEffect_P5_Init();
	LCD_Init();
	__enable_irq();					// Enable interrupts
}

/**
 * Timer32.1 ISR to update speedometers
 */
void T32_INT1_IRQHandler(void) {
	TIMER32_1->INTCLR = 0;	// Reset interrupt flag

	if (Motor_Spinning) {
		Update_Gauges = 1;	// Set flag to update speedometers, reset in main
		Motor_Spinning = 0;	// Reset motor flag, set when hall sensor triggers
	} else if (Speed > 0) {
		Speed = RPM = 0;	// Set Speed and RPM to 0 if motor stopped
		Update_Gauges = 1;	// Set flag to update speedometers, reset in main
	}
}

/**
 * Write current speed to LCD display
 */
void ST7735_WriteSpeedRPM(void) {
	// Character color
	uint16_t white = ST7735_Color565(255, 255, 255);

	// Integer portion of RPM and Speed
	uint16_t rpm_int = (uint16_t) RPM, speed_int = (uint16_t) Speed;

	// Character for each digit
	char rpm_100 = rpm_int > 99 ? rpm_int / 100 + '0' : ' ',
		rpm_10 = rpm_int > 9 ? (rpm_int / 10) % 10 + '0' : ' ',
		rpm_1 = rpm_int % 10 + '0',
		rpm_dec = (RPM - rpm_int) * 10 + '0',
		speed_100 = speed_int > 99 ? speed_int / 100 + '0' : ' ',
		speed_10 = speed_int > 9 ? (speed_int / 10) % 10 + '0' : ' ',
		speed_1 = speed_int % 10 + '0',
		speed_dec = (Speed - speed_int) * 10 + '0';

	// Character of each digit last time
	static char rpm_100_last, rpm_10_last, rpm_1_last, rpm_dec_last,
		speed_100_last, speed_10_last, speed_1_last, speed_dec_last;

	// Write RPM of digits that changed
	if (rpm_100 != rpm_100_last) ST7735_DrawChar(1, 27, rpm_100, white, 0, 4);
	if (rpm_10 != rpm_10_last) ST7735_DrawChar(23, 27, rpm_10, white, 0, 4);
	if (rpm_1 != rpm_1_last) ST7735_DrawChar(45, 27, rpm_1, white, 0, 4);
	if (rpm_dec != rpm_dec_last) ST7735_DrawChar(76, 27, rpm_dec, white, 0, 4);

	// Write Speed of digits that changed
	if (speed_100 != speed_100_last) ST7735_DrawChar(1, 77, speed_100, white, 0, 4);
	if (speed_10 != speed_10_last) ST7735_DrawChar(23, 77, speed_10, white, 0, 4);
	if (speed_1 != speed_1_last) ST7735_DrawChar(45, 77, speed_1, white, 0, 4);
	if (speed_dec != speed_dec_last) ST7735_DrawChar(76, 77, speed_dec, white, 0, 4);

	// Record characters
	rpm_100_last = rpm_100;
	rpm_10_last = rpm_10;
	rpm_1_last = rpm_1;
	rpm_dec_last = rpm_dec;
	speed_100_last = speed_100;
	speed_10_last = speed_10;
	speed_1_last = speed_1;
	speed_dec_last = speed_dec;
}

/**
 * Initialize hall effect inputs on P5.4 - 5.5
 */
void HallEffect_P5_Init(void) {
	P5->SEL0 &= ~0x30;
	P5->SEL1 &= ~0x30;					// P5.4 - 5.5 to GPIO
	P5->DIR &= ~0x30;					// Input
	P5->REN |= 0x30;					// Enable pull resistors
	P5->OUT |= 0x30;					// Pull up selected
	P5->IES &= ~0x30;					// Trigger on rising edge
	P5->IE |= 0x30;						// Enable interrupts

	NVIC_SetPriority(PORT5_IRQn, 7);	// Priority 7
    NVIC_EnableIRQ(PORT5_IRQn);			// Enable Port 5 interrupts
}

/**
 * Port 5 ISR
 * Record time between interrupts to calculate speed
 */
void PORT5_IRQHandler(void) {
	// Period between interrupts on P5.4 - 5.5
	static short period_4 = 0, period_5 = 0;
	// Time of last interrupt on P5.4 - 5.5
	static uint32_t last_4 = 4294967295, last_5 = 4294967295;
	// Current value of Timer32.2
	uint32_t current = TIMER32_2->VALUE;

	if (P5->IFG & BIT4) {
		period_4 = last_4 - current;
		last_4 = current;
		Motor_Spinning = 1;

		// RPM = (60 sec * 187500 * 2 / (14 magnets * 20 gear ratio)) / sum of periods
		RPM = 80357.0 / (period_4 + period_5);
		// MPH = (71 mph / 0.36 sec / 60 sec) * RPM
		Speed = 0.426 * RPM;
	}

	if (P5->IFG & BIT5) {
		period_5 = last_5 - current;
		last_5 = current;
		Motor_Spinning = 1;

		// RPM = (60 sec * 187500 * 2 / (14 magnets * 20 gear ratio)) / sum of periods
		RPM = 80357.0 / (period_4 + period_5);
		// MPH = (71 mph / 0.36 sec / 60 sec) * RPM
		Speed = RPM * 0.426;
	}

	P5->IFG = 0;	// Reset Port 5 interrupt flag
}

/**
 * Initialize PWM on P5.7 using TimerA2.2
 */
void Motor_P5_TA2_2_Init(void) {
	P5->SEL0 |= 0x80;
	P5->SEL1 &= ~0x80;				// P5.7 to TimerA2.2
	P5->DIR |= 0x80;				// Output

	TIMER_A2->CCR[0] = 1800;		// Period, 15 kHz
	TIMER_A2->CCR[2] = 0;			// Duty cyrcle
	TIMER_A2->CCTL[2] = 0xE0;		// Reset/set mode
	TIMER_A2->CTL = 0x0216;			// 10 0001 0100
}

/**
 * Initialize ADC on P6.0
 */
void Potentiometer_P6_A5_Init(void) {
	P6->SEL0 |= 0x01;
	P6->SEL1 |= 0x01;					// P6.0 to ADC14 pin function

	// 10000100 00010100 00110011 10010000
	// /4, /4, ACLK, repeat-single-channel, 32 cycle sample, auto, ON
	// ADC14CLK = 8192 / 4 / 4 = 512 Hz, conversion freq = 512 / 32 = 16 Hz
	ADC14->CTL0 = 0x44D43390;
	ADC14->CTL1 = 0x00050002;			// MEM5, low-power
	ADC14->MCTL[5] = 0x0F;				// A5 to input channel
	ADC14->IER0 |= 0x20;				// Enable A5 interrupt
	ADC14->CTL0 |= 0x03;				// Start conversion

	NVIC_SetPriority(ADC14_IRQn, 7);	// Priority 7
	NVIC_EnableIRQ(ADC14_IRQn);			// Enable ADC14 interrupts
}

/**
 * ADC14 ISR
 * Update PWM based on voltage across potentiometer. (16 Hz)
 */
void ADC14_IRQHandler(void) {
	if (ADC14->IFGR0 & BIT5) {
		TIMER_A2->CCR[2] = ADC_PWM_Vector[ADC14->MEM[5]];
	}
	ADC14->CLRIFGR0 = 0;	// Reset interrupt flag
}
