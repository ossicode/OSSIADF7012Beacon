/*
 * ossi_beacon.c
 *
 *  Created on: 2013. 1. 21.
 *      Author: OSSI
 */
#include "ossi_beacon.h"

#define BEACON_NORMAL_MODE		(0)
#define BEACON_STAND_ALONE_MODE	(1)

uint8_t obcData[OSSI_DATA_SIZE]={0};

#define BEACON_PACKET_SIZE		(64)
uint8_t beaconPacket[BEACON_PACKET_SIZE] ={0};

static const uint8_t hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

#define OSSI_HELLO_PACKET_SIZE (16)
static const uint8_t ossiBeaconHello[OSSI_HELLO_PACKET_SIZE]={'D','E',' ','O','S','S','I','1',' ','A','N','Y','O','U','N','G'};

static volatile uint8_t beaconMode;

void beacon_makePacket(uint8_t num);
uint8_t beacon_morseSend(void);

void beacon_portSetup(void)
{
	// default GPIO setup
	// Some pins are for LFXT1CLK P2SEL = 0xCo
	// DO NOT MAKE ALL PINS TO GPIO!!!!!!
	P1SEL = 0x00;
	P2SEL = 0xC0;
	P3SEL = 0x00;

	// Set all to input
	P1DIR = 0x00;
	P2DIR = 0x00;
	P3DIR = 0x00;

	// set all pin data to 0
	P1OUT = 0x00;
	P2OUT = 0x00;
	P3OUT = 0x00;

	// set proper directions for the pins
	P1DIR = 0xFF & ~(BEACON_MUXOUT_PIN + BEACON_TXCLK_PIN); // ADF7012 TXCLK output is high. Beware!!! Why??
	P2DIR = 0xFF & ~(ADC0_PIN+ADC1_PIN+PA_FAULT_PIN);
	P3DIR = 0xFF & ~(I2C_SDA_PIN+I2C_SCL_PIN+BEACON_CWCONTROL_PIN+UART_RXD_PIN); // make sure I2C pins are input by default
}

void beacon_wdtSetup(void)
{
	wdt_hold();
	// TODO: set wdt reset time
}

void beacon_clockSetup(void)
{
	// default: MCLK=SMCLK=Calibrated 8MHz DCO / ACLK= LFXT1CLK = 32.768kHz
	clock_setup();
	clock_dividerSetup(MCLK_DIVIDED_BY_1, SMCLK_DIVIDED_BY_1, ACLK_DIVIDED_BY_1);
}

void beacon_systimerSetup(void)
{
	// set for 1ms tick / 1 sec tick / compensate msTick every second
	systimer_init(TIMER_A1_ACLK, TIMER_A1_DIVIDED_BY_1, TIMER_A1_UP_MODE, 33, 32765);
	systimer_start();
}

void beacon_init(void)
{
	wdt_hold();
	// as soon as stopping the watchdog timer
	// set up port to protect MCU
	beacon_portSetup();

	// default: MCLK=SMCLK=Calibrated 8MHz DCO / ACLK= LFXT1CLK = 32.768kHz
	clock_setup();
	clock_dividerSetup(MCLK_DIVIDED_BY_1, SMCLK_DIVIDED_BY_1, ACLK_DIVIDED_BY_1);

	// set for 1ms tick / 1 sec tick / compensate msTick every second
	systimer_init(TIMER_A1_ACLK, TIMER_A1_DIVIDED_BY_1, TIMER_A1_UP_MODE, 33, 32765);
	systimer_start();

	adf7012_portSetup();
	adc10_portSetup(ADC10_PIN_2_0 + ADC10_PIN_2_1);
	adc10_init(ADC10_CLOCKSOURCE_SMCLK,ADC10_CLOCKDIVIDER_8,ADC10_CYCLEHOLD_16_CYCLES);

	// for debugging or gps
	uart_initACLK9600();
	uart_start();

	// for external watchdog timer
	P2OUT &= ~EXTWDT_PIN;
	P2DIR |= EXTWDT_PIN;

	// for debugging
	P3OUT &= ~LED_PIN;
	P3DIR |= LED_PIN;

	// initi beacon data and status
	volatile uint8_t i;
	for(i = 0; i< OSSI_DATA_SIZE ; i++)
	{
		obcData[i] = 0;
	}

	for (i = 0; i< BEACON_PACKET_SIZE; i++)
	{
		beaconPacket[i] = 0;
	}

	// i2c slave start
	i2c_portSetup();
	i2c_slaveInit(0x49, OSSI_DATA_SIZE, obcData);
	i2c_slaveStart();
}

void beacon_setExtWdt(uint8_t val)
{
	if(val > 0)
	{
		P2OUT |= EXTWDT_PIN;
	}
	else
	{
		P2OUT &= ~EXTWDT_PIN;
	}
}

void beacon_setExtWdtToggle(void)
{
	P2OUT ^= EXTWDT_PIN;
}

uint8_t beacon_getMorseStatus(void)
{
	return morse_getStatus();
}

uint8_t beacon_morseSend(void)
{
	uint8_t result;
	morse_init();
	morse_send(beaconPacket);

	if (result == ERROR) {return result;}
	return SUCCESS;
}

uint8_t beacon_morseStop(void)
{
	morse_stop();
}

void beacon_updateOBCData(uint8_t intAddr, uint8_t value)
{
	obcData[intAddr] = value;
}

uint8_t beacon_getOBCData(uint8_t intAddr)
{
	return obcData[intAddr];
}

void beacon_taskSchedulePeriod(uint8_t sec)
{
	systimer_setWakeUpPeriod(sec);
	systimer_startWakeUpPeriod();
}

void beacon_taskSchedule(void)
{
	beacon_setExtWdtToggle();
	P3OUT ^= LED_PIN;

	if (beacon_getOBCData(BEACON_CMD1_ADDR) == MORSE_SEND_START)
	{
		// when command is received,
		// let OBC know that the command is read
		beacon_updateOBCData(BEACON_CMD1_ADDR, BEACON_CMD1_CLEAR);

		volatile uint8_t packetNum;

		for (packetNum = 0 ; packetNum < 6 ; packetNum++)
		{
			beacon_makePacket(packetNum);
			beacon_updateOBCData(BEACON_TX_STATUS_ADDR, SENDING);
			beacon_morseSend();
			beacon_updateOBCData(BEACON_TX_STATUS_ADDR, SENT);
		}
		// stop morse after sending all packets
		beacon_morseStop();
		beacon_updateOBCData(BEACON_TX_STATUS_ADDR, SENT);
	}

}

uint8_t beacon_healthCheck(void)
{
	uint8_t result;
	uint16_t adcValue;

	adc10_setVolReference(ADC10_REF_VCC_VSS);
	// check VBUS
	adcValue =  adc10_readChannel(0);
	beacon_updateOBCData(BEACON_VBUS_ADDR, (adcValue >> 2) & 0xFF);
	// if VBUS -> do something
	// check ADC1
	adcValue =  adc10_readChannel(1);
	beacon_updateOBCData(BEACON_ADC1_ADDR, (adcValue >> 2) & 0xFF);
	// if ADC1 -> do something
	// check beacon MCU temp.
	adcValue =  adc10_readChannel(10);
	beacon_updateOBCData(BEACON_TEMP_ADDR, (adcValue >> 2) & 0xFF);
	// adc10_offInternalVolReference(); // turn of reference if internal reference is used

	// check ADF PLL


	if (result == ERROR) {return result;}
	return SUCCESS;
}

void beacon_makePacket(uint8_t num)
{
	#define HEADER_SIZE	(3)

	uint8_t packetHeader[HEADER_SIZE]={'O','S',0};
	// Header Number
	packetHeader[HEADER_SIZE-1] = num;

	volatile uint8_t i;
	// clear previous beacon Packet
	for (i = 0; i< BEACON_PACKET_SIZE; i++)
	{
		beaconPacket[i] = 0;
	}

	// add packet number
	for( i = 0 ; i < HEADER_SIZE; i++)
	{
		beaconPacket[i] = packetHeader[i];
	}

	switch(num)
	{
	case 0:
		// OSSI Anyoung PACKET
		for (i = 0 ; i < OSSI_HELLO_PACKET_SIZE ; i++)
		{
			beaconPacket[i+HEADER_SIZE] = ossiBeaconHello[i];
		}
		break;
	case 1:
		// STATUS DATA PACKET
		for( i = 0 ; i < STATUS_DATA_SIZE; i++)
		{
			// change 1 HEX to 2 ASCII
			beaconPacket[2*i+HEADER_SIZE] = hex[(obcData[i+STATUS_DATA_ADDR] >> 4) & 0x0F]; // Upper
			beaconPacket[2*i+1+HEADER_SIZE] = hex[obcData[i+STATUS_DATA_ADDR] & 0x0F]; // Lower
		}
		break;
	case 2:
		// VI DATA PACKET
		for( i = 0 ; i < VI_DATA_SIZE; i++)
		{
			// change 1 HEX to 2 ASCII
			beaconPacket[2*i+HEADER_SIZE] = hex[(obcData[i+VI_DATA_ADDR] >> 4) & 0x0F]; // Upper
			beaconPacket[2*i+1+HEADER_SIZE] = hex[obcData[i+VI_DATA_ADDR] & 0x0F]; // Lower
		}
		break;
	case 3:
		// TEMP DATA PACKET
		for( i = 0 ; i < TEMP_DATA_SIZE; i++)
		{
			// change 1 HEX to 2 ASCII
			beaconPacket[2*i+HEADER_SIZE] = hex[(obcData[i+TEMP_DATA_ADDR] >> 4) & 0x0F]; // Upper
			beaconPacket[2*i+1+HEADER_SIZE] = hex[obcData[i+TEMP_DATA_ADDR] & 0x0F]; // Lower
		}
		break;
	case 4:
		// MODULE DATA PACKET
		for( i = 0 ; i < MODULE_DATA_SIZE; i++)
		{
			// change 1 HEX to 2 ASCII
			beaconPacket[2*i+HEADER_SIZE] = hex[(obcData[i+MODULE_DATA_ADDR] >> 4) & 0x0F]; // Upper
			beaconPacket[2*i+1+HEADER_SIZE] = hex[obcData[i+MODULE_DATA_ADDR] & 0x0F]; // Lower
		}
		break;
	case 5:
		// LEDTIME DATA PACKET
		for( i = 0 ; i < LEDTIME_DATA_SIZE; i++)
		{
			// change 1 HEX to 2 ASCII
			beaconPacket[2*i+HEADER_SIZE] = hex[(obcData[i+LEDTIME_DATA_ADDR] >> 4) & 0x0F]; // Upper
			beaconPacket[2*i+1+HEADER_SIZE] = hex[obcData[i+LEDTIME_DATA_ADDR] & 0x0F]; // Lower
		}
		break;
	default:
		break;
	}
}

void beacon_setOpMode(uint8_t mode)
{
	beaconMode = mode;
}

uint8_t beacon_getOpMode(uint8_t mode)
{
	return beaconMode;
}

