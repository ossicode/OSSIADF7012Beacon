/*
 * flash.h
 *
 *  Created on: 2013. 1. 16.
 *      Author: OSSI
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "ossi_beacon.h"

// This flash module use Segment C as main
// and Segment D for buffering
// So All result are stored to Segment C
// And read from Segment D

// DO NOT CHANGE THIS!!!
// Using Information Memory Section
// To store system status
// SEG C 64 Bytes
// SEG D 64 Bytes
//
// DO NOT USE SEG A as SEG A contains calibration data!!!!!
//
// Use below address for boundary check


#define FLASH_SEG_C_START_ADDR	(0x1040)
#define FLASH_SEG_C_LAST_ADDR	(0x107F)

#define FLASH_SEG_D_START_ADDR	(0x1000)
#define FLASH_SEG_D_LAST_ADDR	(0x103F)

#define FLASH_SEG_C_SIZE		(64)
#define FLASH_SEG_D_SIZE		(64)

#define FLASH_ACLK 				FSSEL_0                /* Flash clock select: 0 - ACLK */
#define FLASH_MCLK				FSSEL_1                /* Flash clock select: 1 - MCLK */
#define FLASH_SMCLK				FSSEL_2                /* Flash clock select: 2 - SMCLK */
#define FLASH_SMCLK2			FSSEL_3                /* Flash clock select: 3 - SMCLK */

uint8_t flash_writeBegin(uint8_t flashClockSelect, uint8_t flashClockDivider);
uint8_t flash_writeData(uint8_t address, uint8_t byteCount, uint8_t *data);
uint8_t flash_writeEnd(void);

uint8_t flash_readData(uint8_t address, uint8_t byteCount, uint8_t *data);

#endif /* FLASH_H_ */
