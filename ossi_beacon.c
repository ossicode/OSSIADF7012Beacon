/*
 * ossi_beacon.c
 *
 *  Created on: 2013. 1. 21.
 *      Author: OSSI
 */

#include "ossi_beacon.h"


#define BEACON_MAIN_MODE		(0)
#define BEACON_STAND_ALONE_MODE	(1)

#define BEACON_DATA_SIZE (80)
static uint8_t beaconData[80]={0};
static uint8_t beaconPacket[64] ={0};

static volatile uint8_t beaconMode;
static volatile uint8_t beaconPacketNum;

void beacon_portSetup(void)
{
	// default GPIO setup
	// set every pins to GPIO
	P1SEL = 0x00;
	P2SEL = 0x00;
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

	volatile uint8_t i;
	for(i = 0; i< BEACON_DATA_SIZE ; i++)
	{
		becaonData[i] = 0;
	}

	// init beacon Status
	beaconData[BEACON_PLL_LOCKED_ADDR] = PLL_NOT_LOCKED;
	beaconData[BEACON_PA_FAULT_ADDR] = PA_NOT_FAULT;
	beaconData[BEACON_MORSE_STATUS_ADDR] = MORSE_STAND_BY;

	// i2c slave start
	i2c_portSetup();
	i2c_slaveInit(0x49, 64, beaconData);
	i2c_slaveStart();
}

void beacon_taskSchedule(void)
{

	if (beaconData[OBC_BEACON_CMD1_ADDR] == MORSE_SEND_START)
	{
		beaconData[OBC_BEACON_CMD1_ADDR] = OBC_CMD1_CLEAR;
		beaconData[BEACON_MORSE_STATUS_ADDR] = MORSE_PACKET_0_SENDING;
		beaconPacketNum = 0;
	}
	beaconPacketNum++;

}

uint8_t beacon_healthCheck(void)
{
	uint8_t result;
	// check ADF PLL


	if (result == ERROR) {return result;}
	return SUCCESS;
}

void beacon_makePacket(void)
{
	uint8_t result;
	uint16_t adcValue[3];

	adc10_setVolReference(ADC10_REF_VCC_VSS);
	// check VBUS
	adcValue[0] =  adc10_readChannel(0);
	beaconData[BEACON_VBUS_DATA0_ADDR] = adcValue[0] & 0xFF; // LSB
	beaconData[BEACON_VBUS_DATA1_ADDR] = (adcValue[0] >> 8) & 0xFF; // MSB
	// if VBUS -> do something
	// check ADC1
	adcValue[1] =  adc10_readChannel(1);
	beaconData[BEACON_ADC1_DATA0_ADDR] = adcValue[1] & 0xFF; // LSB
	beaconData[BEACON_ADC1_DATA1_ADDR] = (adcValue[1] >> 8) & 0xFF; // MSB
	// if ADC1 -> do something
	// check beacon MCU temp.
	adcValue[2] =  adc10_readChannel(10);
	beaconData[BEACON_TEMP_DATA0_ADDR] = adcValue[2] & 0xFF; // LSB
	beaconData[BEACON_TEMP_DATA1_ADDR] = (adcValue[2] >> 8) & 0xFF; // MSB
	// adc10_offInternalVolReference(); // turn of reference if internal reference is used

	switch(beaconPacketNum)
	{
	case 0:
		break;
	default:
		break;
	}

}

uint8_t beacon_morseSend(void)
{
	uint8_t result;
	morse_send(beaconPacket);
	if (result == ERROR) {return result;}
	return SUCCESS;
}

void beacon_setOpMode(uint8_t mode)
{
	beaconMode = mode;
}

uint8_t beacon_getOpMode(uint8_t mode)
{
	return beaconMode;
}

