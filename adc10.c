/*
 * adc.c
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */
#include "adc10.h"

void adc10_setup(void)
{
	ADC10CTL0 = ADC10ON  + ADC10SHT_2; // ADC10ON,  sampling rate = ACLK * 16
	ADC10CTL1 |= ADC10SSEL_1;                     // ADC clock = ACLK, default: Single channel single conversion
	ADC10AE0 |= 0x03;                             // set input for pin P2.0 , P2.1, no need to set directions
}

void adc10_init(void)
{

}


void adc10_start(uint16_t chan)
{

	// channel number  starts from 0
//	ADC10CTL0 |= ADC10IE; // enable ADC10 interrupt
	ADC10CTL1 |= chan; // choose channel for ADC conversion
	ADC10CTL0 |= ENC + ADC10SC; // starts conversion
	// go into LPM and wait for the interrupt
}


uint16_t adc10_read(void)
{
	// read only after waking up from LPM by ADC interrupt
	return ADC10MEM;
}

void adc10_enable_int(void)
{
	ADC10CTL0 |= ADC10IE;
}

void adc10_disable_int(void)
{
	ADC10CTL0 &= ~ADC10IE;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{

	__bic_SR_register_on_exit(LPM3_bits);        // Clear LPM3_bits from 0(SR)
}
