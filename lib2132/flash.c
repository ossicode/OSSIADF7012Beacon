/*
 * flash.c
 *
 *  Created on: 2013. 1. 16.
 *      Author: OSSI
 */

#include "flash.h"

static volatile uint8_t flashStatus;

uint8_t flash_writeBegin(uint8_t flashClockSelect, uint8_t flashClockDivider)
{
	// TODO: check boundary for the below
	// flashClockSelect value 0~3
	// flashClockDivider value 0~63

	// Set flash timing frequency from 257kHz to 476kHz.
	// TODO: how to act when clock system is changed????

	// Stop Watchdog timer
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
	FCTL2 = FWKEY + flashClockSelect + flashClockDivider;
//	FCTL2 = FWKEY + FSSEL0 + FN0;             // MCLK/2 for Flash Timing Generator

 return 1;
}

uint8_t flash_writeEnd(void)
{
	// TODO: Enable WDT if system needs

	return 1;
}

uint8_t flash_writeData(uint8_t address, uint8_t byteCount, uint8_t *data)
{
	// Erase D
	// Copy C to D Except data we want to modify
	// Erase C
	// write data we want to modify
	// copy D data to C except modified data position


	// TODO: check boundary condition
	// address 0 ~ 63 / byteCount 1 ~ 64
	if((FLASH_SEG_C_START_ADDR + address + byteCount -1 ) > FLASH_SEG_C_LAST_ADDR )
	{
		return 0;
	}

	// TODO: check system voltage
	// We don't check system voltage separately as we use TPS3838 voltage supervisor externally

	_DINT();
	uint8_t *Flash_ptrC;
	uint8_t *default_ptrC;
	uint8_t *Flash_ptrD;
	uint8_t *default_ptrD;
	uint8_t *data_ptr;
	volatile uint8_t i;

	Flash_ptrC =default_ptrC= (uint8_t *) (FLASH_SEG_C_START_ADDR);
	Flash_ptrD =default_ptrD= (uint8_t *)(FLASH_SEG_D_START_ADDR);
	data_ptr = data;

	// Erase D
	FCTL1 = FWKEY + ERASE;                    // Set Erase bit
	FCTL3 = FWKEY;                            // Clear Lock bit
	*Flash_ptrD = 0;

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY + LOCK;

	// Copy C to D Except data we want to modify
	FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
	FCTL3 = FWKEY;                            // Clear Lock bit

	for (i=0; i < FLASH_SEG_D_SIZE; i++)
	{
		if((Flash_ptrC < default_ptrC+ address) || (Flash_ptrC > default_ptrC + address + byteCount-1))
		{
		  *Flash_ptrD = *Flash_ptrC;                   // Write value to flash
		}
	Flash_ptrD++;
	Flash_ptrC++;

	}

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY + LOCK;                     // Set LOCK bit


	Flash_ptrC = default_ptrC;              // Initialize Flash pointer
	Flash_ptrD =default_ptrD;             // Initialize Flash segment B pointer

	// Erase C
	FCTL1 = FWKEY + ERASE;                    // Set Erase bit
	FCTL3 = FWKEY;                            // Clear Lock bit
	*Flash_ptrC = 0;

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY + LOCK;                     // Set LOCK bit

	// Write Data We want to modify
	Flash_ptrC = (uint8_t *)(FLASH_SEG_C_START_ADDR + address);              // Initialize Flash pointer

	FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
	FCTL3 = FWKEY;                            // Clear Lock bit

	for (i=0; i < byteCount; i++)
	{
		*Flash_ptrC++ = *data_ptr++;                   // Write value to flash
	}

	Flash_ptrC = default_ptrC;              // Initialize Flash pointer
	Flash_ptrD =default_ptrD;             // Initialize Flash segment B pointer

	// Copy D to C except modified data position
	for (i=0; i < FLASH_SEG_C_SIZE; i++)
	{
		if((Flash_ptrD < default_ptrD + address) || (Flash_ptrD > default_ptrD + address + byteCount-1))
		{
		  *Flash_ptrC = *Flash_ptrD;                   // Write value to flash
		}
		Flash_ptrD++;
		Flash_ptrC++;
	}

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY + LOCK;
	_EINT();
	return 1;
}

uint8_t flash_readData(uint8_t address, uint8_t byteCount, uint8_t *data)
{
	uint8_t* flash_ptr;
	uint8_t* data_ptr;
	volatile uint8_t i;

	if((FLASH_SEG_C_START_ADDR + address + byteCount -1 ) > FLASH_SEG_C_LAST_ADDR )
	{
		return 0;
	}

	flash_ptr = (uint8_t *)(FLASH_SEG_C_START_ADDR + address);
	data_ptr = (uint8_t *)data;

	for(i = 0; i < byteCount; i++)
	{
		*data_ptr = *flash_ptr;
		flash_ptr++;
		data_ptr++;
	}

	return 1;
}
