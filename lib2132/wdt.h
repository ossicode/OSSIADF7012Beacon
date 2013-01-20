/*
 * wdt.h
 *
 *  Created on: 2013. 1. 17.
 *      Author: OSSI
 */

#ifndef WDT_H_
#define WDT_H_

#include "msp430f2132.h"
#include "ossi_types.h"
#include "debug.h"

#define WDT_CLOCKSOURCE_SMCLK  (0)
#define WDT_CLOCKSOURCE_ACLK   (WDTSSEL)

#define WDT_CLOCKDIVIDER_32768	(0)
#define WDT_CLOCKDIVIDER_8192	(WDTIS0)
#define WDT_CLOCKDIVIDER_512	(WDTIS1)
#define WDT_CLOCKDIVIDER_64		(WDTIS0 + WDTIS1)


void wdt_hold(void);
void wdt_init(uint8_t clockSelect, uint8_t clockDividerSelect);
void wdt_start(void);
void wdt_reset(void);

#endif /* WDT_H_ */
