/*
 * i2c.h
 *
 *  Created on: 2012. 12. 30.
 *      Author: OSSI
 */

#ifndef I2C_H_
#define I2C_H_

#include "ossi_beacon.h"
#include "timerA.h"
#include "uart.h"

extern volatile uint8_t uart_rxByte;


/*
 * I2C Master Status
 */
#define I2C_IDLE				(0)
#define I2C_NACK				(1)
#define I2C_TIMEOUT				(2)
#define I2C_BUS_NOT_READY		(3)
#define I2C_BUS_POWERDOWN		(4)
#define	I2C_TRANSFER_DONE		(5)
#define I2C_TRANSFER_TIMEOUT 	(6)
#define I2C_ACK					(7)
#define I2C_ARB_LOST			(8)

#define I2C_MASTER_RX_BUFFER_SIZE 16
#define I2C_MASTER_TX_BUFFER_SIZE 16

#define I2C_SLAVE_BUFFER_SIZE 64
#define I2C_SLAVE_PACKET_SIZE 8

#define I2C_CLOCKSOURCE_ACLK    UCSSEL_1
#define I2C_CLOCKSOURCE_SMCLK   UCSSEL_2

#define I2C_NACK_INT			UCNACKIE                        /* NACK Condition interrupt enable */
#define I2C_STOP_INT			UCSTPIE                         /* STOP Condition interrupt enable */
#define I2C_START_INT			UCSTTIE                         /* START Condition interrupt enable */
#define I2C_ARB_LOST_INT		UCALIE                          /* Arbitration Lost interrupt enable */

#define I2C_TRANSMIT_MODE       UCTR
#define I2C_RECEIVE_MODE        0x00

void i2c_portSetup(void);

//void i2c_enable(void);
//void i2c_disable(void);
void i2c_reset(void);

void i2c_enableTXInterrupt(void);
void i2c_disableTXInterrupt(void);
void i2c_enableRXInterrupt(void);
void i2c_disableRXInterrupt(void);
void i2c_enableSubInterrupt(uint8_t interruptSelect);
void i2c_disableSubInterrupt(uint8_t interruptSelect);
void i2c_disableAllInterrupt(void);

uint8_t i2c_generalCall(void);
uint8_t i2c_ackPolling(uint8_t slaveAddress);

void i2c_masterInit(uint8_t selctClockSource, uint16_t preScalerValue ,uint8_t modeSelect);
uint8_t i2c_masterWrite(uint8_t slaveAddress, uint8_t byteCount, uint8_t *data);
uint8_t i2c_masterRead(uint8_t slaveAddress, uint8_t byteCount, uint8_t *data);
uint8_t i2c_masterWriteAndRead(uint8_t slaveAddress, uint8_t txByteCount, uint8_t *txData, uint8_t rxByteCount, uint8_t *rxData);

void i2c_slaveInit(uint8_t ownAddress, uint8_t byteCount, uint8_t *data);
void i2c_slaveStart(void);

uint8_t i2c_getInternalAddress(void);

uint8_t i2c_getSlaveTxDone(void);
void i2c_setSlaveTxDone(uint8_t status);
uint8_t i2c_getSlaveRxDone(void);
void i2c_setSlaveRxDone(uint8_t status);
uint8_t i2c_getGeneralCallReceived(void);
void i2c_setGeneralCallReceived(uint8_t status);

void i2c_busRecovery(void);

#endif /* I2C_H_ */
