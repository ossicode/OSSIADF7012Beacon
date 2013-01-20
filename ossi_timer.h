/*
 * ossi_timer.h
 *
 *  Created on: 2013. 1. 19.
 *      Author: OSSI
 */

#ifndef OSSI_TIMER_H_
#define OSSI_TIMER_H_

#include "ossi_beacon.h"

#define TIMER_A1_TACLK			TASSEL_0                	 /* Timer A clock source select: 0 - TACLK */
#define TIMER_A1_ACLK			TASSEL_1                     /* Timer A clock source select: 1 - ACLK  */
#define TIMER_A1_SMCLK			TASSEL_2                     /* Timer A clock source select: 2 - SMCLK */
#define TIMER_A1_INCLK			TASSEL_3                     /* Timer A clock source select: 3 - INCLK */

#define TIMER_A1_DIVIDED_BY_1	ID_0                   	     /* Timer A input divider: 0 - /1 */
#define TIMER_A1_DIVIDED_BY_2	ID_1                         /* Timer A input divider: 1 - /2 */
#define TIMER_A1_DIVIDED_BY_4	ID_2                         /* Timer A input divider: 2 - /4 */
#define TIMER_A1_DIVIDED_BY_8	ID_3                         /* Timer A input divider: 3 - /8 */

#define TIMER_A1_STOP			MC_0                         /* Timer A mode control: 0 - Stop */
#define TIMER_A1_UP_MODE		MC_1                         /* Timer A mode control: 1 - Up to CCR0 */
#define TIMER_A1_CONT_MODE		MC_2                         /* Timer A mode control: 2 - Continous up */
#define TIMER_A1_UPDOWN_MODE	MC_3                         /* Timer A mode control: 3 - Up/Down */

void systimer_init(uint16_t timerBSourceSelect, uint8_t timerBDividerSelect, uint8_t timerBMode, uint16_t timerBMsThreshold ,uint16_t timerBSecThreshold);
void systimer_start(void);
void systimer_stop(void);
uint32_t systimer_getMsTick(void);
uint32_t systimer_getSecTick(void);

#endif /* OSSI_TIMER_H_ */
