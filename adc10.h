/*
 * adc10.h
 *
 *  Created on: 2012. 12. 27.
 *      Author: OSSI
 */

#ifndef ADC10_H_
#define ADC10_H_

#include "ossibeacon.h"

void adc10_setup(void);
void adc10_init(void);
void adc10_start(uint16_t chan);
uint16_t adc10_read(void);
void adc10_enable_int(void);
void adc10_disable_int(void);

#endif /* ADC10_H_ */
