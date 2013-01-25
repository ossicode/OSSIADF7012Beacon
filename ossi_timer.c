/*
 * ossi_timer.c
 *
 *  Created on: 2013. 1. 19.
 *      Author: OSSI
 */
#include "ossi_timer.h"

static volatile uint32_t sysMsTick = 0;
static volatile uint32_t sysSecTick = 0;
static volatile uint16_t sysMsDelayTick = 0;
static volatile uint8_t sysMsDelayOn = 0;
static volatile uint16_t sysMsDelay = 0;
static volatile uint8_t sysSecWakeOn = 0;
static volatile uint16_t sysSecWakePeriod = 0;
static volatile uint16_t sysSecWakeTick = 0;

static uint16_t msUnit;

void systimer_init(uint16_t timerBSourceSelect, uint8_t timerBDividerSelect, uint8_t timerBMode, uint16_t timerBMsThreshold ,uint16_t timerBSecThreshold)
{

	// Clear the timer
	TA1CTL = TACLR;
	// set tick unit time
	TA1CCR0 = timerBSecThreshold;
	TA1CCR1 = msUnit = timerBMsThreshold;
//	msUnit = timerBMsThreshold;
	// set divider first
	TA1CTL = timerBDividerSelect;
	// set timer source
	TA1CTL |= timerBSourceSelect;
	// set timer mode
	TA1CTL |= timerBMode;
}

void systimer_start(void)
{
	// init sysMsTick counter
	sysMsTick = 0;
	sysSecTick = 0;

	// reset timer B counter
	TA1R = 0;

	TA1CCTL0 &= ~CCIFG;
	TA1CCTL0 |= CCIE;

	TA1CCTL1 &= ~CCIFG;
	TA1CCTL1 |= CCIE;
}

void systimer_stop(void)
{
	// disable interrupt to stop sysMsTick from increasing

	TA1CCTL1 &= ~CCIE;
	TA1CCTL1 &= ~CCIFG;

	TA1CCTL0 &= ~CCIE;
	TA1CCTL0 &= ~CCIFG;

	// Timer Stop
	TA1CTL &= ~MC_3;
	TA1CTL |= MC_0;

	// reset timer B counter
	TA1R = 0;

	// init sysMsTick counter
	sysMsTick = 0;
	sysSecTick = 0;
}

uint32_t systimer_getMsTick(void)
{
//	sysMsTick = TA1R & 0xFFFF;
//	sysMsTick = (uint32_t)(sysMsTick / msUnit);
//	sysMsTick = sysMsTick + sysSecTick*1000;
	return sysMsTick;
}

uint32_t systimer_getSecTick(void)
{
	return sysSecTick;
}

void systimer_msDelay(uint16_t msDelay)
{
	sysMsDelay = msDelay;
	sysMsDelayTick = 0;
	sysMsDelayOn = 1;
	while(sysMsDelayOn);
//	volatile uint32_t prevMsTick, currMsTick;
//	prevMsTick = systimer_getMsTick();
//	while(1)
//	{
//		currMsTick= systimer_getMsTick();
//		if((currMsTick - prevMsTick) > msDelay)
//			break;
//	}
}

void systimer_setWakeUpPeriod(uint16_t sec)
{
	sysSecWakePeriod = sec;
}

void systimer_startWakeUpPeriod(void)
{
	sysSecWakeTick = 0;
	sysSecWakeOn = 1;
}

void systimer_stopWakeUpPeriod(void)
{
	sysSecWakeOn = 0;
	sysSecWakeTick = 0;
}

// Timer1_A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void systimer_sec(void)
{
	// TODO: overflow check needed?
//	P3OUT ^= LED_PIN;
	sysSecTick++;
	sysMsTick = sysSecTick * 1000;
	TA1CCR1 = msUnit;
	if (sysSecWakeOn)
	{
		sysSecWakeTick++;
		if (sysSecWakeTick >= sysSecWakePeriod)
		{
			sysSecWakeTick = 0;
			__bic_SR_register_on_exit(LPM3_bits);
		}
	}
}

// Timer1_A1 interrupt service routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void systimer_ms(void)
{
	switch(TA1IV)
	{
	case TA1IV_TACCR1:
		// TODO: overflow check needed?
//		P3OUT ^= BEACON_CWCONTROL_PIN;
		sysMsTick++;
		TA1CCR1 += msUnit;
		if(sysMsDelayOn)
		{
			sysMsDelayTick++;
			if (sysMsDelayTick >= sysMsDelay)
			{
				sysMsDelayOn = 0;
			}
		}
		break;
	default:
		break;
	}
}
