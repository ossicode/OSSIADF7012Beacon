/*
 * adc.c
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */
#include "adc10.h"


void adc10_portSetup(uint8_t ports)
{
	ASSERT(ports <= 0xFF);
	// reset ports
	ADC10AE0 = ports;                             // set input for pin P2.0 , P2.1, no need to set directions
}

/*
 * single channel single conversion only setting
 */
void adc10_init(uint8_t clockSourceSelect, uint8_t clockSourceDivider, uint16_t clockCycleHoldCount)
{
    ASSERT(clockSourceSelect <= ADC10_CLOCKSOURCE_SMCLK);
    ASSERT(clockSourceDivider <= ADC10_CLOCKDIVIDER_8);
    ASSERT(clockCycleHoldCount <= ADC10_CYCLEHOLD_64_CYCLES);

	// make sure clear ENC to set registers
	ADC10CTL0 &= ~ENC;

	// Turn OFF ADC10 Module & Clear Interrupt Registers
	ADC10CTL0 &= ~(ADC10ON  + ENC + ADC10SC + ADC10IE + ADC10IFG);      // ADC10ON,  sampling rate = ACLK * 16


	// Reset and Set ADC10 Control 1
	// Using ADC10_SAMPLEHOLDSOURCE_SC only
	ADC10CTL1 = ADC10_SAMPLEHOLDSOURCE_SC + clockSourceSelect + clockSourceDivider;

	//Reset and Set ADC10 Control 0
	ADC10CTL0 = clockCycleHoldCount;
}

/*
 * we limit adc10_setVolReference() function to 2 options:
 * 1. Vr+: VCC, Vr-: VSS / SREFx: 000
 * 2. Vr+: Vref+ , Vr-: VSS / SREFx: 001
 * for Vref+ reference voltage setting, we use:
 * ADC10SR = 0
 * REFOUT = 0
 * REFBIRST = 0
 * REF2_5V = 1 (2.5V internal Reference Voltage)
 * REFON = 1
 */
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
	 ADC10CTL0 |= (ADC10_REF_VREF_VSS + REF2_5V + REFON);
	volatile uint8_t i;
	for(i = 0; i < 17; i++)
	{
		__delay_cycles(7372);
	}
	}
}

void adc10_offInternalVolReference(void)
{
	// clear ENC to set registers
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 &= ~(ADC10SR + REFOUT + REFBURST + REF2_5V + REFON);
}

/*
 * single channel / single conversion
 * param: channelSelect ranges from 0~11
 *
 */

uint16_t adc10_readChannel(uint8_t channelSelect)
{
	ASSERT(inputSourceSelect <= 11);
	uint16_t adc10Channel = 0;;
	uint16_t adc10Value = 0;
	// convert channelSelect to set register
	adc10Channel = (adc10Channel | channelSelect) << 12;
	adc10Channel = (adc10Channel & 0xF000);

	 // clear ENC to set registers
	 ADC10CTL0 &= ~ENC;

	 // reset input channel and conversion sequence mode
	 ADC10CTL1 &= 0x0FF9;

	 // set input channel
	 ADC10CTL1 |= adc10Channel;
	 // turn adc10 on
	 ADC10CTL0|= ADC10ON;
	 // clear IFG Before Starting Conversion
	 ADC10CTL0 &= ~ADC10IFG;
	 ADC10CTL0 |= ENC;
	 // set to start conversion
	 ADC10CTL0 |= ADC10SC;

	 // wait adc10 to complete the conversion
	 while((ADC10CTL0 & ADC10IFG)==0);
	 adc10Value = ADC10MEM;

	 // clear ENC when ADC10 is not busy
	 while(ADC10CTL0 &ADC10BUSY);
	 // clear IFG after the Conversion
	 ADC10CTL0 &= ~ENC;
	 ADC10CTL0 &= ~ADC10IFG;

	 return adc10Value;
}


