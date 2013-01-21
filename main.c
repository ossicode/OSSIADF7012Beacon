/*
 * main.c
 */

#include "ossi_beacon.h"

void beacon_taskScheduler(void);
void beacon_dataReceive(void);
void beacon_dataProcessing(void);
void beacon_dataSend(void);

void main(void)
{
	beacon_init();

	while(1)
	{

		// Enter LPM3, interrupts enabled
		__bis_SR_register(LPM3_bits + GIE);

		// beacon wakes up after receiving data from OBC
		beacon_taskSchedule();
		beacon_makePacket();
		beacon_morseSend();

//		if(i2c_getSlaveRxDone())
//		{
//			i2c_setSlaveRxDone(0);
//			if(beaconData[2] == 0x03)
//			{
//				morse_init();
////				morse_send(hojunData);
//			}
//		}
//		beacon_dataReceive();
//		beacon_dataProcessing();
//		beacon_dataSend();
	}
}

void beacon_taskScheduler(void)
{

}

void beacon_dataReceive(void) // uart related handler
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

void beacon_dataProcessing(void)
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

void beacon_dataSend(void) // timer0 related handler
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
