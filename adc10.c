/*
 * adc.c
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */
#include "adc10.h"

void adc10_setup(uint8_t ports)
{
	ASSERT(ports <= 0xFF);
	// reset ports
	ADC10AE0 = ports;                             // set input for pin P2.0 , P2.1, no need to set directions
}

void adc10_init(uint16_t sampleHoldSignalSourceSelect,
		uint8_t clockSourceSelect,
		uint8_t clockSourceDivider,
		uint16_t clockCycleHoldCount, uint8_t MSCbit)
{
	//Make sure the ENC bit is cleared before initializing the ADC10
	ASSERT(!(ADC10CTL0 & ENC));

    ASSERT(sampleHoldSignalSourceSelect <= ADC10_SAMPLEHOLDSOURCE_3);
    ASSERT(clockSourceSelect <= ADC10_CLOCKSOURCE_SMCLK);
    ASSERT(clockSourceDivider <= ADC10_CLOCKDIVIDER_8);
    ASSERT(clockCycleHoldCount <= ADC10_CYCLEHOLD_64_CYCLES);

    //Turn OFF ADC10 Module & Clear Interrupt Registers
	ADC10CTL0 &= ~(ADC10ON  + ENC + ADC10SC + ADC10IE + ADC10IFG);      // ADC10ON,  sampling rate = ACLK * 16

	//Reset and Set ADC10 Control 0
	ADC10CTL0 = clockCycleHoldCount + MSCbit;

	//Reset and Set ADC10 Control 1
	ADC10CTL1 = sampleHoldSignalSourceSelect + clockSourceSelect + clockSourceDivider;                     // ADC clock = ACLK, default: Single channel single conversion
}

void adc10_startConversion(uint16_t inputSourceSelect, uint8_t conversionSequenceModeSelect)
{
	 ASSERT(inputSourceSelect <= ADC10_INPUT_VMID);
	 ASSERT(conversionSequenceModeSelect <= ADC10_REPEATED_SEQOFCHANNELS);

	 // clear ENC to set registers
	 ADC10CTL0 &= ~ENC;

	 //TODO: reset input channel

	 // set input channel
	 ADC10CTL1 |= inputSourceSelect;

	 // set to start conversion
	 ADC10CTL0 |= ADC10SC  + ENC;

}

void adc10_startDTCConversion(uint16_t inputSourceSelect, uint8_t conversionSequenceModeSelect, uint16_t blockStartAddress)
{
	 ASSERT(inputSourceSelect <= ADC10_INPUT_VMID);
	 ASSERT(conversionSequenceModeSelect <= ADC10_REPEATED_SEQOFCHANNELS);
	 //TODO:how can we check blockStartAddress is valid or not. need to check the address is even number.

	 // clear ENC to set registers
	 ADC10CTL0 &= ~ENC;

	 //TODO: reset input channel

	 // set input channel
	 ADC10CTL1 |= inputSourceSelect;

	 // reset the ADC10SA address to start DTC conversion again
	 ADC10SA = blockStartAddress;

	 ADC10CTL0 |= ADC10SC  + ENC;
}

void adc10_enable(void)
{
	ADC10CTL0 |= ADC10ON;
}

void adc10_disable(void)
{
	ADC10CTL0 &= ~ADC10ON;
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
