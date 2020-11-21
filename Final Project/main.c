/********************************************
 * Author: Scott VandenToorn
 * Date: October 23, 2020
 * Program: EGR 326-903 Final Project - Dashboard
 * Description:
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
#include "Rotary.h"
#include "DS3231_RTC.h"
#include "Motor.h"
#include "Stepper.h"
#include "Proximity.h"

/* Function prototypes */
void Init(void);

/* Global variables */
volatile uint8_t Update_Gauges = 0;			// True when speedometers/tachometers ready to be updated
volatile uint8_t Update_LCD_A_Scroll = 0;	// True when LCD should scroll left one digit
volatile uint8_t Read_RTC = 1;				// True when time and temp on RTC should be read
volatile uint16_t Backup_Distance = 0;		// Proximity sensor distance (inches)
const uint16_t ADC_PWM_Vector[256] = {		// Vector for converting a 255-bit ADC14 reading to PWM duty cycle
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
	WDT_A->CTL = 0x5A80;					// Stop watchdog timer

	uint8_t selected_option = 0;			// Selected menu option on LCD A
	unsigned char rtc_time_day_response[3];	// Response from RTC

	Init();

	while(1) {
		if (Rotary_Button | Rotary_CW | Rotary_CCW) {
			if (Rotary_Button && LCD_A_State == 0x00) {	// 0x00 = scroll state
				LCD_A_State++;
				selected_option = 0;
				LCD_A_Menu_Show();
				LCD_A_Menu_MoveCursor(selected_option);
			}
			else if (LCD_A_State == 0x01) {				// 0x01 = main menu state
				if (Rotary_Button) {
					ST7735_FillScreen(0xFFFF, LCD_A_CS);
					switch(selected_option) {
						case 0:	// Alarm Log
							break;
						case 1:	// Set Time
							break;
						case 2:	// Exit
							LCD_A_State = 0x00;
							break;
					}
				} else {
					// Increment/decrement selected menu option
					if (Rotary_CCW) selected_option += selected_option < 2 ? 1 : -2;
					if (Rotary_CW) selected_option += selected_option > 0 ? -1 : 2;
					LCD_A_Menu_MoveCursor(selected_option);
				}
			}
			else if (LCD_A_State == 0x10) {				// 0x10 = alarm log state

			}
			else if (LCD_A_State == 0x11) {				// 0x11 = update rtc time state

			}

			// Reset flags
			Rotary_Button = 0;
			Rotary_CW = 0;
			Rotary_CCW = 0;
		}

		if (Read_RTC) {
			// Read day and time on RTC
			RTC_I2C1_BurstRead(RTC_ADDR, 1, 3, rtc_time_day_response);
			// Set text for LCD scroll
			LCD_A_Scroll_SetText(rtc_time_day_response, RTC_Temperature());
			Read_RTC = 0;						// Reset flag
		}

		if (Update_Gauges) {
			SevenSeg_WriteFloat(0, 3, Speed);	// Update 7-segment
			LCD_B_WriteSpeedRPM(Speed, RPM);	// Update LCD
			Update_Gauges = 0;					// Reset flag
		}

		if (Update_LCD_A_Scroll && LCD_A_State == 0x00) {
			LCD_A_Scroll();						// Scroll LCD day/time/temp text
			Update_LCD_A_Scroll = 0;			// Reset flag
		}

    	if (Stepper_Interrupt) {
    		Stepper_Step = (int) RPM * 2;		// Set desired stepper position
    		Stepper_Update();					// Update stepper
    		Stepper_Interrupt = 0;				// Reset flag
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

    Clock_Frequencies_Init();
//	SYSCTL->WDTRESET_CTL |= 0x00;	// WDT Soft reset
//	WDT_A->CTL = 0x5A2C;			// Start WDT, 4s timeout
	SysTick_Init();
	Timer32_1_Interrupt_Init(3);	// ?? Hz interrupt
	Timer32_2_Counter_Init();
	TimerA0_0_Interrupt_s_Init(2);	// 0.5 Hz interrupt
	TimerA1_0_Interrupt_ms_Init(2);	// 512 Hz interrupt
	LCD_Init();
	Motor_PWM_ADC_Init();
	SevenSeg_P2_UCA1_Init();
	Rotary_P3_GPIO_Init();
	RTC_I2C1_Init();
	Stepper_P7_Init();				// Initialize stepper GPIO
	Proximity_GPIO_P4_Init();
	__enable_irq();					// Enable interrupts
}


/********************************************
 * Interrupts
 ********************************************/

/**
 * Timer32.1 periodic ISR
 */
void T32_INT1_IRQHandler(void) {
	TIMER32_1->INTCLR = 0;					// Reset interrupt flag

	Update_Gauges = 1;						// Flag to update speedometers, reset in main
	Update_LCD_A_Scroll = 1;				// Flag to update scrolling time on LCD A, reset in main

	if (Motor_Spinning) Motor_Spinning = 0;	// Reset motor flag, set when hall sensor triggers
	else Speed = RPM = 0;					// Set Speed and RPM to 0 if motor stopped
}

/**
 * TimerA0.0 periodic ISR
 */
void TA0_0_IRQHandler(void) {
	TIMER_A0->CCTL[0] &= ~0x01;	// Reset interrupt flag
	Read_RTC = 1;				// Flag to read RTC module, reset in main
	Proximity_Trigger();		// Trigger proximity sensor
}

/**
 * TimerA1_0 periodic ISR (512 Hz)
 */
void TA1_0_IRQHandler(void) {
	TIMER_A1->CCTL[0] &= ~0x01;	// Reset interrupt flag
	Stepper_Interrupt = 1;		// Set flag, reset in main
}

/**
 * ADC14 ISR (around 16 Hz)
 * Update PWM duty cycle for motor and LCD brightness based on
 * voltage across potentiometer or photoresistor
 */
void ADC14_IRQHandler(void) {
	// GL5528 photoresistor ADC interrupt
	if (ADC14->IFGR0 & BIT4) {
		// Calculate and update duty cycle for LCD brightness PWM
		int duty = 1800 - (ADC14->MEM[4] - 60) * 9;
		TIMER_A2->CCR[1] = duty > 200 ? duty : 200;
	}

	// Motor slide potentiometer ADC interrupt
	if (ADC14->IFGR0 & BIT5) {
		// Calculate and update duty cycle for motor PWM
		TIMER_A2->CCR[2] = ADC_PWM_Vector[ADC14->MEM[5]];
	}

	ADC14->CLRIFGR0 = 0;	// Reset interrupt flag
}
