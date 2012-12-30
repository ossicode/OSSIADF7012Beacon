/*
 * adc.c
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */
#include "adc10.h"
//	works for repeated and DTC ADC10 conversion for now 30 Dec 2012
//
// 	USE ADC module something like this
//	main()
//	{
//	adc10_portSetup()
//		while(1)
//		{
//			adc10_init() // init everytime using ADC to save power consumption
//			adc10_enable()
//			adc10_enableInterrupt()
//			adc10_setVolReference()
//			adc10_startConversion()
//			Low Power Mode
//		}
//	}
//
//	ADC_ISR
//	{
//		clear ENC // clear ENC as soon as ISR is entered to avoid corruption of data and sequence
//		adc10_disable()
//		adc10_disableInterrupt()
//		exit Low Power Mode
//	}


void adc10_portSetup(uint8_t ports)
{
	ASSERT(ports <= 0xFF);
	// reset ports
	ADC10AE0 = ports;                             // set input for pin P2.0 , P2.1, no need to set directions
}

// we initialize adc10 with MSC bit = 1 as we are going to repeated single channel mode and don't want to intervene during the conversion
// TODO: check default values and make sure you understand everything
void adc10_init(uint16_t sampleHoldSignalSourceSelect,
		uint8_t clockSourceSelect,
		uint8_t clockSourceDivider,
		uint16_t clockCycleHoldCount)
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
	ADC10CTL0 = clockCycleHoldCount + ADC10_MULTIPLESAMPLESENABLE;

	//Reset and Set ADC10 Control 1
	ADC10CTL1 = sampleHoldSignalSourceSelect + clockSourceSelect + clockSourceDivider;                     // ADC clock = ACLK, default: Single channel single conversion
}

// 	we limit adc10_setVolReference() function to 2 options:
//	1. Vr+: VCC, Vr-: VSS / SREFx: 000
//	2. Vr+: Vref+ , Vr-: VSS / SREFx: 001
//	for Vref+ reference voltage setting, we use:
//	ADC10SR = 0
//	REFOUT = 0
//	REFBIRST = 0
//	REF2_5V = 1 (2.5V internal Reference Voltage)
//	REFON = 1
void adc10_setVolReference(uint16_t refVoltageSourceSelect)
{
	ASSERT(refVoltageSourceSelect <= ADC10_REF_VREF_VSS);
	 // clear ENC to set registers
	 ADC10CTL0 &= ~ENC;

	 // reset voltage reference settings
	 // clear SREFx bits
	 ADC10CTL0 &= 0x1FFF;
	 // clear rest of the reference settings
	 ADC10CTL0 &= ~(ADC10SR + REFOUT + REFBURST + REF2_5V + REFON);

	 if(refVoltageSourceSelect == ADC10_REF_VCC_VSS)
	 {
		 ADC10CTL0 |= ADC10_REF_VCC_VSS;
	 }
	 else
	 {
		 ADC10CTL0 |= ADC10_REF_VREF_VSS + REF2_5V + REFON;
	 }
}

// we only use DTC options for ADC
// TODO:for now, only ADC10_REPEATED_SINGLECHANNEL / ADC10_REPEATED_SEQOFCHANNELS modes are working!!!
// So declare buffer and use the buffer address and the size as the parameter of adc10_startConversion()
// TODO: check the function for single-channel / single conversion mode and sequences of channel mode.
void adc10_startConversion(uint16_t inputSourceSelect, uint8_t conversionSequenceModeSelect, uint16_t blockStartAddress, uint8_t buf_size)
{
	 ASSERT(inputSourceSelect <= ADC10_INPUT_VMID);
	 ASSERT(conversionSequenceModeSelect <= ADC10_REPEATED_SEQOFCHANNELS);
	 //TODO:how can we check blockStartAddress is valid or not. need to check the address is even number.
	 ASSERT(buf_size <= 0x80); // we're going to limit maximum buffer size to 128

	 // clear ENC to set registers
	 ADC10CTL0 &= ~ENC;

	 // reset input channel and conversion sequence mode
	 ADC10CTL1 &= 0x0FF9;

	 // set input channel and conversion sequence mode
	 ADC10CTL1 |= inputSourceSelect + conversionSequenceModeSelect;

	 //TODO: we are not using polling but we clear ENC bit inside ADC ISR. check whether this is universal to all modes
//	 while (ADC10CTL1 & ADC10BUSY);

	 // reset DTC by writing 0 to ADC10DTC1 register
	 ADC10DTC1 = 0;

	 // set ADC10DTC1 size
	 ADC10DTC1 = buf_size;

	 // reset the ADC10SA address to start DTC conversion again
	 ADC10SA = blockStartAddress;

	 // set to start conversion
	 ADC10CTL0 |= ADC10SC  + ENC;
}

// how to stop conversion? stop conversion has to take care of all 4 modes. but now this is only valid for 2 repeated mode with MSC bit =1
void adc10_stopConversion(void)
{
	//TODO: we are not using polling but we clear ENC bit inside ADC ISR. check whether this is universal to all modes
	// while (ADC10CTL1 & ADC10BUSY);
	// clear ENC to stop the conversion
	ADC10CTL0 &= ~ENC;

}

void adc10_enable(void)
{
	ADC10CTL0 |= ADC10ON;
}

void adc10_disable(void)
{
	// turn off ADC core and Reference to save power consumption
//	ADC10CTL0 &= ~(ADC10SR + REFOUT + REFBURST + REF2_5V + REFON);
//	ADC10CTL0 &= ~ADC10ON;

	// turn off all for low power operation
	ADC10CTL0 =0;
}


void adc10_enableInterrupt(void)
{
	ADC10CTL0 |= ADC10IE;
}

void adc10_disableInterrupt(void)
{
	ADC10CTL0 &= ~ADC10IE;
}

//#pragma vector=ADC10_VECTOR
//__interrupt void ADC10_ISR(void)
//{
//
//
//	__bic_SR_register_on_exit(LPM3_bits);        // Clear LPM3_bits from 0(SR)
//}
