/*
 * wdt.c
 *
 *  Created on: 2013. 1. 17.
 *      Author: OSSI
 */

#include "wdt.h"

void wdt_hold(void)
{
	volatile uint8_t wdtStatus = (WDTCTL & 0x0F) | WDTHOLD;
	// Stop watchdog timer
	WDTCTL = WDTPW + wdtStatus ;
}

void wdt_init(uint8_t clockSelect, uint8_t clockDividerSelect)
{
	// reset wdt counter, wdt stop
	// reset wdt clock, wdt clock dividier
	WDTCTL = WDTPW + WDTCNTCL + WDTHOLD+ clockSelect + clockDividerSelect;
}

void wdt_start(void)
{
	volatile uint8_t wdtStatus = (WDTCTL & 0x0F) & ~(WDTHOLD);
	// Start watchdog timer
	WDTCTL = WDTPW + wdtStatus ;
}

void wdt_reset(void)
{
	// reset wdt counter
	volatile uint8_t wdtStatus = (WDTCTL & 0x0F) | WDTCNTCL;
	WDTCTL = WDTPW + wdtStatus ;
}
