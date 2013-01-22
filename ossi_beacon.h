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

/*
 * Beacon DATA Memory Map
 * 0 ~ 63: Shared OBC Data
 * 64~ 79: Beacon Data & OBC Command
 */

#define OBC_LATCH_UP_DATA_ADDR	(0)
#define OBC_LATCH_UP_DATA_SIZE 	(6)

#define OBC_EPS_DATA_ADDR		(6)
#define OBC_EPS_DATA_SIZE		(7)

#define OBC_ADC_DATA_ADDR		(13)
#define OBC_ADC_DATA_SIZE 		(26)

#define OBC_TEMP_DATA_ADDR		(39)
#define OBC_TEMP_DATA_SIZE 		(14)

#define OBC_TIME_DATA_ADDR		(53)
#define OBC_TIME_DATA_SIZE 		(8)

#define BEACON_VBUS_DATA0_ADDR		(64)
#define BEACON_VBUS_DATA1_ADDR		(65)
#define BEACON_ADC1_DATA0_ADDR		(66)
#define BEACON_ADC1_DATA1_ADDR		(67)
#define BEACON_TEMP_DATA0_ADDR		(68)
#define BEACON_TEMP_DATA1_ADDR		(69)
#define BEACON_PLL_LOCKED_ADDR		(70)
#define BEACON_PA_FAULT_ADDR		(71)
#define	BEACON_MORSE_STATUS_ADDR	(72)
#define OBC_BEACON_CMD1_ADDR		(78)
#define OBC_BEACON_CMD2_ADDR		(79)

/*
 * Beacon Status
 */
// BEACON_PLL_LOCKED_ADDR
#define	PLL_NOT_LOCKED			(0)
#define PLL_LOCKED				(128)

// BEACON_PA_FAULT_ADDR
#define PA_NOT_FAULT			(0)
#define PA_FAULT				(128)

// BEACON_MORSE_STATUS_ADDR
#define MORSE_STAND_BY			(0)
#define MORSE_PACKET_0_SENDING	(128)
#define MORSE_PACKET_1_SENDING	(129)
#define MORSE_PACKET_2_SENDING	(130)
#define MORSE_PACKET_3_SENDING	(131)
#define MORSE_PACKET_4_SENDING	(132)
#define MORSE_PACKET_5_SENDING	(133)
#define MORSE_PACKET_6_SENDING	(134)
#define MORSE_SEND_DONE			(135)

// OBC_BEACON_CMD1_ADDR
#define OBC_CMD1_CLEAR			(0)
#define MORSE_SEND_START		(128)
#define MORSE_SEND_STOP			(129)


//typedef union {
//    uint8_t data[OBC_LATCH_UP_DATA_SIZE];
//    struct {
//        uint8_t obcFault; // active low
//        uint8_t commsFault; // active low
//        uint8_t beaconFault; // active low
//        uint8_t ledFault; // active low
//        uint8_t commsPaFault; // active low
//        uint8_t beaconPaFault; // active low
//    } u8;
//} obcLatchUpData_t;
//
//typedef union {
//    uint8_t data[OBC_EPS_DATA_SIZE];
//    struct {
//        uint8_t solarOn; // active low
//        uint8_t batOn; // active low
//        uint8_t scOn; // active low
//        uint8_t batChrg; // active low
//        uint8_t batFault; // active low
//        uint8_t scPfg; // active low
//        uint8_t scoutPg; // active low
//    } u8;
//} obcEpsData_t;
//
//typedef union {
//    uint8_t data[OBC_ADC_DATA_SIZE];
//    struct {
//        uint8_t vsolar[2];
//        uint8_t vbat[2];
//        uint8_t vsupercap1[2];
//        uint8_t vsupercap2[2];
//        uint8_t vbus[2];
//        uint8_t vbeaconPa[2];
//        uint8_t solarCs1MinusZ[2];
//        uint8_t solarCs2MinusY[2];
//        uint8_t solarCs3MinusX[2];
//        uint8_t solarCs4PlusZ[2];
//        uint8_t solarCs5PlusY[2];
//        uint8_t sunSensor[2];
//        uint8_t beaconTemp[2];
//    } u16;
//} obcAdcData_t;
//
//typedef union {
//    uint8_t data[OBC_TEMP_DATA_SIZE];
//    struct {
//        uint8_t obc[2];
//        uint8_t comms[2];
//        uint8_t minusZ[2];
//        uint8_t minusY[2];
//        uint8_t minusX[2];
//        uint8_t plusZ[2];
//        uint8_t plusY[2];
//    } u16;
//} obcTempData_t;
//
//typedef union {
//    uint8_t data[OBC_TIME_DATA_SIZE];
//    struct {
//        uint8_t hour;
//        uint8_t min;
//        uint8_t sec;
//        uint8_t year;
//        uint8_t month;
//        uint8_t padding0;
//        uint8_t padding1;
//        uint8_t padding2;
//    } u8;
//} obcTimeData_t;


void beacon_portSetup(void);
void beacon_init(void);

#endif /* OSSIBEACON_H_ */
