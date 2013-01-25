/*
 * ossibeacon.h
 *
 *  Created on: 2012. 12. 22.
 *      Author: OSSI
 */

#ifndef OSSIBEACON_H_
#define OSSIBEACON_H_

#include "msp430f2132.h"
#include "ossi_types.h"
#include "ossi_timer.h"
#include "ossi_morse.h"
#include "ossi_gps.h"
#include "clock.h"
#include "system.h"
#include "util.h"
#include "printf.h"
#include "adf7012.h"
#include "uart.h"
#include "wdt.h"
#include "i2c.h"
#include "adc10.h"
#include "debug.h"
#include "ossi_data.h"

#define ERROR 0
#define SUCCESS 1

/*
 * OSSI Beacon Pin Settings
 */

// PORT 1
#define BEACON_SCLK_PIN				(BIT0)
#define BEACON_MUXOUT_PIN			(BIT1)
#define BEACON_TXCLK_PIN			(BIT2)
#define BEACON_DATA_PIN				(BIT3)

// PORT 2
#define ADC0_PIN					(BIT0)
#define ADC1_PIN					(BIT1)
#define EXTWDT_PIN 					(BIT2)
#define BEACON_SLE_PIN				(BIT3)
#define BEACON_SDATA_PIN			(BIT4)
#define PA_FAULT_PIN				(BIT5)

// PORT 3
#define LED_PIN 					(BIT0)
#define I2C_SDA_PIN 				(BIT1)
#define I2C_SCL_PIN 				(BIT2)
#define BEACON_CWCONTROL_PIN		(BIT3)
#define UART_TXD_PIN				(BIT4)
#define UART_RXD_PIN				(BIT5)
#define PA_ON_PIN					(BIT6)
#define BEACON_CE_PIN				(BIT7)

void beacon_init(void);
void beacon_taskSchedulePeriod(uint8_t sec);
void beacon_taskSchedule(void);

#endif /* OSSIBEACON_H_ */
