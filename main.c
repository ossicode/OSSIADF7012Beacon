/*
 * main.c
 */

#include "ossi_beacon.h"
#include "printf.h"
#include "adf7012.h"
#include "uart.h"
#include "wdt.h"
#include "ossi_morse.h"
#include "ossi_gps.h"
#include "i2c.h"

uint8_t beaconData[64]={0};
uint8_t hojunData[20]={'H','O','J','U','N',' ','I','S',' ','B','E','S','T'};

void beacon_data_receive(void);
void beacon_data_processing(void);
void beacon_data_send(void);

void main(void)
{
	wdt_hold();
	clock_setup();
	clock_dividerSetup(MCLK_DIVIDED_BY_1, SMCLK_DIVIDED_BY_1, ACLK_DIVIDED_BY_1);

	P1DIR = 0xFF & ~(BIT1+BIT2); // ADF7012 TXCLK output is high. Beware!!! Why??
	P1OUT = 0x00;
	P3DIR = 0xFF & ~(BIT1+BIT2+BIT3+BIT5);
	P3OUT = 0x00;
	P2DIR = 0xFF & ~(BIT0+BIT1+BIT5);
	P2OUT = 0x00;

	P2OUT &= ~EXTWDT_PIN;
	P2DIR |= EXTWDT_PIN;

	P3OUT &= ~LED_PIN;
	P3DIR |= LED_PIN;

	// set for 1ms tick / 1 sec tick / compensate msTick every second
	systimer_init(TIMER_A1_ACLK, TIMER_A1_DIVIDED_BY_1, TIMER_A1_UP_MODE, 33, 32765);
	systimer_start();

	adf7012_setup();

	uart_initACLK9600();
	uart_start();

//	_EINT();
	i2c_portSetup();
	i2c_slaveInit(0x49, 64, beaconData);
	i2c_slaveStart();

	while(1)
	{
		// Enter LPM3, interrupts enabled
		__bis_SR_register(LPM3_bits + GIE);
		if(i2c_getSlaveRxDone())
		{
			i2c_setSlaveRxDone(0);
			if(beaconData[2] == 0x03)
			{
				morse_init();
				morse_send(hojunData);
			}
		}
//		beacon_data_receive();
//		beacon_data_processing();
//		beacon_data_send();
	}
}

void beacon_data_receive(void) // uart related handler
{
	// gps
	if (uart_rxReady())
	{
		uart_clearRxFlag();
		// uart_get_byte() only works after uart ISR
		// TODO:check possibility of error!!!
		if(gps_updateData(uart_getByte()))
		{
			// if all the gps data we want are received
			gps_setReadyFlag();
			return;
		}
	}
	else
	{
		return;
	}
}

void beacon_data_processing(void)
{
	//gps
	// gps
	if (gps_isReady())
	{
		gps_clearReadyFlag();
		// process gps data
		// make packet from i2c and gps data

		gps_makePacket();
		// ready to send morse code
		morse_init();
	}
	else
	{
		return;
	}
}

void beacon_data_send(void) // timer0 related handler
{
	if(morse_isReady())
	{
		// clear the flag
		morse_clearSendFlag();
		morse_send(gps_getStream());
	}
	else
	{
		return;
	}

}
