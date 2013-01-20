/*
 * timerA.c
 *
 *  Created on: 2013. 1. 12.
 *      Author: OSSI
 */

#include "timerA.h"


volatile uint8_t i2cTimeOut;

// If variable timeout is 10, Timeout is 3.2768 sec.
// If ACLK = 32768Hz
// We want to One tick (ellapsed_time) is 0.03 sec, so CCR0 is 122.88 (CCR0 = 0.03 * 4096(TACLK))


void i2c_timerInit(uint16_t timerASourceSelect, uint8_t timerADividerSelect, uint8_t timerAMode, uint16_t timerAThreshold)
{
	// reset timer A
	TA0CTL = TACLR;

	TA0CCR0 = timerAThreshold;
	TA0CTL = timerADividerSelect;    // ID_3: Timer A input divider: 3 - /8
	TA0CTL |= timerASourceSelect;  // TASSEL_1: Timer A clock source select: 1 - ACLK
	TA0CTL |= timerAMode;   //  MC_1: Timer A mode control: 1 - Up to CCR0 // timer start counting from now
}


void i2c_timerTimeoutStart(void)
{
	// reset timeout variable
	i2cTimeOut = 0;

	TAR = 0;

	// make sure clear timer A flag before enable interrupt
	TA0CCTL0 &= ~CCIFG;
	TA0CCTL0 = CCIE;
}


void i2c_timerTimeoutStop(void)
{
	// disable timer interrupt
	TA0CCTL0 &= ~CCIE;
	TA0CCTL0 &= ~CCIFG;
	// Timer Stop
	TA0CTL |= MC_0;

	TAR= 0;
	i2cTimeOut = 0;

}


// Timer0_A0 interrupt service routine
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void Timer_A (void)
//{
//	// disable timer interrupt
//	TA0CCTL0 &= ~CCIE;
//	i2cTimeOut = 1;
//	P3OUT ^= LED_PIN;
//}

