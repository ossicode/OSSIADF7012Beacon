/*
 * adc10.h
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */

#ifndef ADC10_H_
#define ADC10_H_

#include "ossibeacon.h"

//*****************************************************************************
//
//The following are values that can be passed to adc10_setup()
//
//
//*****************************************************************************
#define ADC10_PIN_2_0		        (BIT0)
#define ADC10_PIN_2_1              	(BIT1)
#define ADC10_PIN_2_2              	(BIT2)
#define ADC10_PIN_2_3              	(BIT3)
#define ADC10_PIN_2_4              	(BIT4)
#define ADC10_PIN_3_0              	(BIT5)
#define ADC10_PIN_3_6              	(BIT6)
#define ADC10_PIN_3_7               (BIT7)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_init() in the
//sampleTimerSourceSelect parameter.
//
//*****************************************************************************
#define ADC10_SAMPLEHOLDSOURCE_SC (SHS_0)
#define ADC10_SAMPLEHOLDSOURCE_1  (SHS_1)
#define ADC10_SAMPLEHOLDSOURCE_2  (SHS_2)
#define ADC10_SAMPLEHOLDSOURCE_3  (SHS_3)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_init() in the
//clockSourceSelect parameter.
//
//*****************************************************************************
#define ADC10_CLOCKSOURCE_ADC10OSC (ADC10SSEL_0)
#define ADC10_CLOCKSOURCE_ACLK     (ADC10SSEL_1)
#define ADC10_CLOCKSOURCE_MCLK     (ADC10SSEL_2)
#define ADC10_CLOCKSOURCE_SMCLK    (ADC10SSEL_3)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_setupSamplingTimer() in
//the clockCycleHoldCount parameter.
//
//*****************************************************************************
#define ADC10_CYCLEHOLD_4_CYCLES    (ADC10SHT_0)
#define ADC10_CYCLEHOLD_8_CYCLES    (ADC10SHT_1)
#define ADC10_CYCLEHOLD_16_CYCLES   (ADC10SHT_2)
#define ADC10_CYCLEHOLD_64_CYCLES   (ADC10SHT_3)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_memoryConfigure() in the
//inputSourceSelect parameter.
//
//*****************************************************************************
#define ADC10_INPUT_A0              (INCH_0)
#define ADC10_INPUT_A1              (INCH_1)
#define ADC10_INPUT_A2              (INCH_2)
#define ADC10_INPUT_A3              (INCH_3)
#define ADC10_INPUT_A4              (INCH_4)
#define ADC10_INPUT_A5              (INCH_5)
#define ADC10_INPUT_A6              (INCH_6)
#define ADC10_INPUT_A7              (INCH_7)
#define ADC10_INPUT_VREF_POS        (INCH_8)
#define ADC10_INPUT_VREF_NEG        (INCH_9)
#define ADC10_INPUT_TEMPSENSOR      (INCH_10)
#define ADC10_INPUT_VMID		  	(INCH_11)
//#define ADC10_INPUT_VMID            (INCH_12)
//#define ADC10_INPUT_VMID            (INCH_13)
//#define ADC10_INPUT_VMID            (INCH_14)
//#define ADC10_INPUT_VMID            (INCH_15)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_init() in the
//clockSourceDivider parameter.
//Note: Bit 8-9 determines if the pre-divider is 1, 4, or 64
//Bits 5-7 determine the the post-divider 1-8 (0x00 - 0xE0)
//
//*****************************************************************************
#define ADC10_CLOCKDIVIDER_1   (ADC10DIV_0)
#define ADC10_CLOCKDIVIDER_2   (ADC10DIV_1)
#define ADC10_CLOCKDIVIDER_3   (ADC10DIV_2)
#define ADC10_CLOCKDIVIDER_4   (ADC10DIV_3)
#define ADC10_CLOCKDIVIDER_5   (ADC10DIV_4)
#define ADC10_CLOCKDIVIDER_6   (ADC10DIV_5)
#define ADC10_CLOCKDIVIDER_7   (ADC10DIV_6)
#define ADC10_CLOCKDIVIDER_8   (ADC10DIV_7)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_startConversion() in the
//conversionSequenceModeSelect parameter.
//
//*****************************************************************************
#define ADC10_SINGLECHANNEL          (CONSEQ_0)
#define ADC10_SEQOFCHANNELS          (CONSEQ_1)
#define ADC10_REPEATED_SINGLECHANNEL (CONSEQ_2)
#define ADC10_REPEATED_SEQOFCHANNELS (CONSEQ_3)

//*****************************************************************************
//
//The following are values that can be passed to ADC10_setupSamplingTimer() in
//the multipleSamplesEnabled parameter.
//
//*****************************************************************************
#define ADC10_MULTIPLESAMPLESDISABLE ( !(MSC) )
#define ADC10_MULTIPLESAMPLESENABLE  (MSC)





void adc10_setup(uint8_t ports);
void adc10_init(uint16_t sampleHoldSignalSourceSelect, uint8_t clockSourceSelect, uint8_t clockSourceDivider);
void adc10_start(uint16_t chan);
uint16_t adc10_read(void);
void adc10_enable_int(void);
void adc10_disable_int(void);

#endif /* ADC10_H_ */
