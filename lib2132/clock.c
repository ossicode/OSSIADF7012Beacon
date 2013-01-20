/*
 * clock.c
 *
 *  Created on: 2013. 1. 16.
 *      Author: OSSI
 */

#include "clock.h"

// main clock will be shared via system
static volatile uint8_t clockMode;

uint8_t clock_getMode(void)
{
	return clockMode;
}

uint8_t clock_setup(void)
{
	// after POR: MCLK = SMCLK = DCO ~ 1.2MHz
	// set DCO speed to calibrated 8MHz
	// NO VLOCLK use

	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;
	// TODO: Capacitor setting for ACLK??
	// 12.5pF internal Capacitor setting
	// got rid of external capacitor now
	BCSCTL3 &= 0x01;
	BCSCTL3 |= XCAP_3;

	return CLOCK_DCO_LFXT1;
}

void clock_dividerSetup(uint8_t MCLKDividerSelect, uint8_t SMCLKDividerSelect, uint8_t ACLKDividerSelect)
{
	// reset divider first

	// reset ACLK divider
	BCSCTL1 &= 0xCF; // Clear BIT5 and BIT4
	// select ACLK Divider
	BCSCTL1 |= ACLKDividerSelect;

	// reset MCLK, SMCLK divider
	BCSCTL2 &= 0x00; // Clear BIT5,BIT4, BIT2, BIT1 and reset as 2132 only use DCO
	// select MCLK Divider
	BCSCTL2 |= MCLKDividerSelect;
	// select SMCLK Divider
	BCSCTL2 |= SMCLKDividerSelect;
}
