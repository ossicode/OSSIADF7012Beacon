/*
 * timerA.h
 *
 *  Created on: 2013. 1. 12.
 *      Author: OSSI
 */

#ifndef TIMERA_H_
#define TIMERA_H_

#include "ossi_beacon.h"

extern volatile uint8_t i2cTimeOut;


//#define DEFAULT_CCR0 2
#define DEFAULT_CCR0 122 // 30ms timeout
//#define DEFAULT_CCR0 244

#define TIMER_A0_TACLK			TASSEL_0                	 /* Timer A clock source select: 0 - TACLK */
#define TIMER_A0_ACLK			TASSEL_1                     /* Timer A clock source select: 1 - ACLK  */
#define TIMER_A0_SMCLK			TASSEL_2                     /* Timer A clock source select: 2 - SMCLK */
#define TIMER_A0_INCLK			TASSEL_3                     /* Timer A clock source select: 3 - INCLK */

#define TIMER_A0_DIVIDED_BY_1	ID_0                   	     /* Timer A input divider: 0 - /1 */
#define TIMER_A0_DIVIDED_BY_2	ID_1                         /* Timer A input divider: 1 - /2 */
#define TIMER_A0_DIVIDED_BY_4	ID_2                         /* Timer A input divider: 2 - /4 */
#define TIMER_A0_DIVIDED_BY_8	ID_3                         /* Timer A input divider: 3 - /8 */

#define TIMER_A0_STOP			MC_0                         /* Timer A mode control: 0 - Stop */
#define TIMER_A0_UP_MODE		MC_1                         /* Timer A mode control: 1 - Up to CCR0 */
#define TIMER_A0_CONT_MODE		MC_2                         /* Timer A mode control: 2 - Continous up */
#define TIMER_A0_UPDOWN_MODE	MC_3                         /* Timer A mode control: 3 - Up/Down */


void i2c_timerInit(uint16_t timerASourceSelect, uint8_t timerADividerSelect, uint8_t timerAMode,uint16_t timerAThreshold);
void i2c_timerTimeoutStop(void);
void i2c_timerTimeoutStart(void);


#endif /* TIMERA_H_ */
