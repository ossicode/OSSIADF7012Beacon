/*
 * i2c.c
 *
 *  Created on: 2012. 12. 30.
 *      Author: OSSI
 */

// we implement I2C as below:
//
// 7 bit addressing mode
// NO Multi-Master

#include "i2c.h"


// Master
static uint8_t *masterTxData;
static uint8_t *masterRxData;

static volatile uint16_t masterTxCurrentCnt;
static volatile uint8_t masterRxCurrentCnt;

static volatile uint8_t i2cMasterStatus;

// Slave
static volatile uint8_t *slaveData;
static volatile uint8_t *defaultSlaveData;

static volatile uint16_t slaveIndex;
static volatile uint16_t slaveBufSize;

static volatile uint8_t i2cInteralAddress;

static volatile uint8_t i2cSlaveTxDone;
static volatile uint8_t i2cSlaveRxDone;

static volatile uint8_t i2cSlaveTxInProgress;
static volatile uint8_t i2cSlaveRxInProgress;

static volatile uint8_t i2cStartDetected;
static volatile uint8_t i2cGeneralCallDetected;


void i2c_portSetup(void)
{
	// For MSP430F2132, P3.1 = SDA, P3.2 = SCL
	P3SEL |= 0x06;
}

//void i2c_enable(void)
//{
//
//}
//
//void i2c_disable(void)
//{
//
//}

void i2c_reset(void)
{
	UCB0CTL1 |= UCSWRST;
	UCB0CTL1 &= ~UCSWRST;
}

void i2c_enableRXInterrupt(void)
{
	IE2 |= UCB0RXIE;
}

void i2c_disableRXInterrupt(void)
{
	IE2 &= ~UCB0RXIE;
}

void i2c_enableTXInterrupt(void)
{
	IE2 |= UCB0TXIE;
}

void i2c_disableTXInterrupt(void)
{
	IE2 &= ~UCB0TXIE;
}

void i2c_enableUCB0Interrupt(uint8_t interruptSelect)
{
	UCB0I2CIE |= interruptSelect;
}

void i2c_disableUCB0Interrupt(uint8_t interruptSelect)
{
	UCB0I2CIE &= ~interruptSelect;
}

void i2c_disableAllInterrupt(void)
{
	IE2 &= ~(UCB0TXIE+UCB0RXIE);
	UCB0I2CIE &= 0xF0;
	UCB0I2COA &= ~UCGCEN;
}

uint8_t i2c_getMasterStatus(void)
{
	return i2cMasterStatus;
}

void i2c_setMasterStatus(uint8_t status)
{
	i2cMasterStatus = status;
}

uint8_t i2c_waitForBusReady(void)
{
	// check bus status
	while (UCB0STAT & UCBBUSY)
	{
		if(i2cTimeOut)
		{
			// when timeout, stop the timeout timer first
			i2c_timerTimeoutStop();

			// check whether BUS is power down
			// change pin function only when I2C bus is busy for timeout period otherwise I2C module will not be functioning
			P3SEL &= ~(I2C_SDA_PIN + I2C_SCL_PIN);			// set pins to GPIO
			P3DIR &= ~(I2C_SDA_PIN + I2C_SCL_PIN);			// set input direction

			// check SDA and SCL are both LOW
			// TODO: checking only one time is enough?
			if (((P3IN & I2C_SDA_PIN) == 0) && ((P3IN &I2C_SDA_PIN) == 0))
			{
				P3SEL |= I2C_SDA_PIN + I2C_SCL_PIN;			// set pins back to I2C function
				// report error
				return I2C_BUS_POWERDOWN;
			}
			else
			{
				P3SEL |= I2C_SDA_PIN + I2C_SCL_PIN;			// set pins back to I2C function
				// report error
				return I2C_BUS_NOT_READY;
			}

		}
	}

	return I2C_IDLE;
}

// TODO: make inline function later
uint8_t i2c_checkArbLost(void)
{
	if (UCB0STAT & UCALIFG)
	{
		i2c_disableAllInterrupt();
		// when NACK is received, stop the timeout timer first
		i2c_timerTimeoutStop();
		// clear NACKIFG flag manually
		UCB0STAT &= ~UCALIFG;
		// while waiting for the NACK, masterTxCurrentCnt will increase as the code is executed
		// so we intentionally set the counter to 0 before return

		return 1;
	}
	else
	{
	return 0;
	}
}

// TODO: make inline function later
uint8_t i2c_checkNACK(void)
{
	if (UCB0STAT & UCNACKIFG)
	{
		i2c_disableAllInterrupt();
		// when NACK is received, stop the timeout timer first
		i2c_timerTimeoutStop();
		// clear NACKIFG flag manually
		UCB0STAT &= ~UCNACKIFG;
		return 1;
	}
	else
	{
	return 0;
	}
}


// TODO: make inline function later
uint8_t i2c_checkTimeout(void)
{
	if(i2cTimeOut)
	{
		i2c_disableAllInterrupt();
		// when timeout, stop the timeout timer first
		i2c_timerTimeoutStop();

		// 1 = timeout
		return 1;
	}
	else
	{
		return 0;
	}
}

// TODO: make inline function later
/*
 *  Use this function only after previous transfer to check whether the transfer is finished
 */
uint8_t i2c_waitForPreviousTransfer(uint8_t byteCount)
{

	// when write transfer is finished, read
	// Check Stop condition is sent
	while (UCB0CTL1 & UCTXSTP)				// check all data are sent
	{
		if(i2cTimeOut)
		{
			// when timeout, stop the timeout timer first
			i2c_timerTimeoutStop();
			return I2C_TRANSFER_TIMEOUT;
		}
	}

	return I2C_TRANSFER_DONE;
}

// call this only when you need need master
void i2c_masterInit(uint8_t selctClockSource, uint16_t preScalerValue ,uint8_t modeSelect)
{
	ASSERT((selctClockSource == I2C_CLOCKSOURCE_ACLK)||(selctClockSource == I2C_CLOCKSOURCE_SMCLK));
	ASSERT((preScalerValue>=4) && (preScalerValue<=0xFFFF));
	ASSERT((modeSelect == I2C_TRANSMIT_MODE)||(modeSelect == I2C_RECEIVE_MODE));

	// reset I2C
	UCB0CTL1 = UCSWRST;
	// reset and set UCB0CTL0 for I2C mode
	// UCA10 = 0 owe address 7 bit
	// UCSLA10 = 0 slave address 7bit
	// UCMM = 0 no multi-master
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
	// reset clock source while UCSWRST = 1
	UCB0CTL1 = selctClockSource + modeSelect + UCSWRST;
	// fSCL = selctClockSource / preScalerValue
	// for single master mode, minimum preScalerValue = 4
	UCB0BR0 = preScalerValue & 0xFF;
	UCB0BR1 = (preScalerValue >> 8) & 0xFF;
	UCB0CTL1 &= ~UCSWRST;
}


uint8_t i2c_generalCall(void)
{
	// disable all i2c interrupts
	i2c_disableAllInterrupt();

	// set default I2C status
	i2cMasterStatus = I2C_IDLE;

	// timer start
	i2c_timerInit(TIMER_A0_ACLK, TIMER_A0_DIVIDED_BY_8, TIMER_A0_UP_MODE , DEFAULT_CCR0); // set i2c timeout timer
	i2c_timerTimeoutStart();

	i2cMasterStatus = i2c_waitForBusReady();
	if (i2cMasterStatus != I2C_IDLE)
	{
		return 0;
	}

	i2c_masterInit(I2C_CLOCKSOURCE_SMCLK, 80,I2C_TRANSMIT_MODE);

	UCB0I2CSA = 0x00;

	// We just want to send 0x00 address
	// So we set START and STOP in the beginning
    UCB0CTL1 &= ~UCTXSTT;
	UCB0CTL1 |=  UCTXSTT;                 	// I2C TX, start condition
	UCB0CTL1 |=  UCTXSTP;					// I2C TX, STOP condition
	// first START condition can only be clear
	// after ACK is received from the slave
	// If the data is not written to the buffer, SCL will be held low waiting for the ACK
	// So we write buffer which will be discarded
	UCB0TXBUF = 0x00;						// Just Not to hold the SCL low
											// Data should NOT be transfered

	// STT check is not used as we only want to send an address
	while(UCB0CTL1 & UCTXSTT)				// wait for START to be transmitted
	{
		// check Arbitration Lost before NACK as when Arbitration is lost you cannot send STOP after NACK
		if(i2c_checkArbLost())
		{
			i2cMasterStatus = I2C_ARB_LOST;
			return 0;
		}
		// check NACK before checking timeout to prevent timeout when we have NACK
		if(i2c_checkNACK())
		{
			i2cMasterStatus = I2C_NACK;
			// TODO: generate STOP condition
			return 0;
		}

		if(i2c_checkTimeout())
		{
			i2cMasterStatus = I2C_TIMEOUT;;
			return 0;
		}
	}

	// And we generate as soon as we put data into buffer
	// this stop will discard the data in the buffer
//	UCB0CTL1 |=  UCTXSTP;					// I2C TX, STOP condition
	IFG2 &= ~UCB0TXIFG;
	while(UCB0CTL1 & UCTXSTP)
	{
		if(i2c_checkTimeout())
		{
			i2cMasterStatus = I2C_TIMEOUT;;
			return 0;
		}
	}

	// After STOP condition is transfered, disable all i2c interrupt
	i2c_disableAllInterrupt();

	// timer stop after sending STOP condition
	i2c_timerTimeoutStop();

	return 1;

}

uint8_t i2c_ackPolling(uint8_t slaveAddress)
{

	// After STOP condition is transfered, disable all i2c interrupt
	i2c_disableAllInterrupt();

	// timer stop after sending STOP condition
	i2c_timerTimeoutStop();

	return 1;
}

uint8_t i2c_masterWrite(uint8_t slaveAddress, uint8_t byteCount, uint8_t *data)
{
	ASSERT((byteCount >=1)&&(byteCount<=I2C_MASTER_TX_BUFFER_SIZE));

	// disable all i2c interrupts
	i2c_disableAllInterrupt();

	masterTxData = data;
	masterTxCurrentCnt = 0;

	// set default I2C status
	i2cMasterStatus = I2C_IDLE;

	// timer start
	i2c_timerInit(TIMER_A0_ACLK, TIMER_A0_DIVIDED_BY_8, TIMER_A0_UP_MODE , DEFAULT_CCR0); // set i2c timeout timer
	i2c_timerTimeoutStart();

	i2cMasterStatus = i2c_waitForBusReady();
	if (i2cMasterStatus != I2C_IDLE)
	{
		return 0;
	}

	i2c_masterInit(I2C_CLOCKSOURCE_SMCLK, 80,I2C_TRANSMIT_MODE);

	// enable interrupt here

	// Clear UCALIFG if you previous transfer ended with arbitration lost
	UCB0STAT &= ~UCALIFG;

	// Clear NACKIFG:
	// This Is Very Important If you Want Ignore NACK from previous transfer
	// and proceed next transfer
	UCB0STAT &= ~UCNACKIFG;

	// slaveAddress is right justified. bit 6 is MSB for 7 bit address
	// TODO:how to check whether we have valid 7 bit slaveAddress?
	UCB0I2CSA = slaveAddress;
	UCB0CTL1 |=  UCTXSTT;                 // I2C TX, start condition

	// check data ACK is received while sending data
	// When NACK is received, NO TXRDYIFG set
	// So the below should not be executed, or timeout will occur
	for(masterTxCurrentCnt = 0 ; masterTxCurrentCnt < byteCount ; masterTxCurrentCnt++)
	{
		while((IFG2 & UCB0TXIFG) == 0)
		{
			// check Arbitration Lost before NACK as when Arbitration is lost you cannot send STOP after NACK
			if(i2c_checkArbLost())
			{
				i2cMasterStatus = I2C_ARB_LOST;
				return 0;
			}
			// check NACK before checking timeout to prevent timeout when we have NACK
			if(i2c_checkNACK())
			{
				i2cMasterStatus = I2C_NACK;
				// TODO: generate STOP condition
				return 0;
			}

			if(i2c_checkTimeout())
			{
				i2cMasterStatus = I2C_TIMEOUT;;
				return 0;
			}
		}

		// writing data and TXRDYIFG cleared
		UCB0TXBUF = masterTxData[masterTxCurrentCnt];
	}

	// Wait for the last byte is ACKed and generate stop condition
	while((IFG2 & UCB0TXIFG) == 0)
	{
		// check Arbitration Lost before NACK as when Arbitration is lost you cannot send STOP after NACK
		if(i2c_checkArbLost())
		{
			i2cMasterStatus = I2C_ARB_LOST;
			return 0;
		}

		// check NACK before checking timeout to prevent timeout when we have NACK
		if(i2c_checkNACK())
		{
			i2cMasterStatus = I2C_NACK;
			// TODO: generate STOP condition
			return 0;
		}

		if(i2c_checkTimeout())
		{
			i2cMasterStatus = I2C_TIMEOUT;;
			return 0;
		}
	}
	UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition right after sending last data

	i2cMasterStatus = i2c_waitForPreviousTransfer(byteCount);
	if (i2cMasterStatus != I2C_TRANSFER_DONE)
	{
		return 0;
	}

	// After STOP condition is transfered, disable all i2c interrupt
	i2c_disableAllInterrupt();

	// timer stop after sending STOP condition
	i2c_timerTimeoutStop();

	// when everything goes well we return final count
	// value 0 means something wrong
	return masterTxCurrentCnt;
}

uint8_t i2c_masterRead(uint8_t slaveAddress, uint8_t byteCount, uint8_t *data)
{
	ASSERT((byteCount >=1)&&(byteCount<=I2C_MASTER_RX_BUFFER_SIZE));

	volatile uint8_t lastCount;

	// disable all i2c interrupts
	i2c_disableAllInterrupt();

	masterRxData = data;
	masterRxCurrentCnt = 0;
	lastCount = byteCount - 1;

	// set default I2C status
	i2cMasterStatus = I2C_IDLE;

	// timer start
	i2c_timerInit(TIMER_A0_ACLK, TIMER_A0_DIVIDED_BY_8, TIMER_A0_UP_MODE , DEFAULT_CCR0); // set i2c timeout timer
	i2c_timerTimeoutStart();

	i2cMasterStatus = i2c_waitForBusReady();
	if (i2cMasterStatus != I2C_IDLE)
	{
		return 0;
	}

	i2c_masterInit(I2C_CLOCKSOURCE_SMCLK, 80, I2C_RECEIVE_MODE);

	// enable interrupt here

	// Clear UCALIFG if you previous transfer ended with arbitration lost
	UCB0STAT &= ~UCALIFG;

	// Clear NACKIFG:
	// This Is Very Important If you Want Ignore NACK from previous transfer
	// and proceed next transfer
	UCB0STAT &= ~UCNACKIFG;

	// slaveAddress is right justified. bit 6 is MSB for 7 bit address
	// TODO:how to check whether we have valid 7 bit slaveAddress?
	UCB0I2CSA = slaveAddress;


	if (byteCount >1)
	{
		// if byteCount > 1
		UCB0CTL1 |= UCTXSTT;					// I2C start condition

		// check data ACK is received while sending data
		// When NACK is received, NO TXRDYIFG set
		// So the below should not be executed, or timeout will occur
		for(masterRxCurrentCnt = 0 ; masterRxCurrentCnt < byteCount ; masterRxCurrentCnt++)
		{
			while((IFG2 & UCB0RXIFG) == 0)
			{
				// check Arbitration Lost before NACK as when Arbitration is lost you cannot send STOP after NACK
				if(i2c_checkArbLost())
				{
					i2cMasterStatus = I2C_ARB_LOST;
					return 0;
				}

				// check NACK before checking timeout to prevent timeout when we have NACK
				if(i2c_checkNACK())
				{
					i2cMasterStatus = I2C_NACK;
					// TODO: generate STOP condition
					return 0;
				}

				if(i2c_checkTimeout())
				{
					i2cMasterStatus = I2C_TIMEOUT;;
					return 0;
				}
			}


			// reading data and RXRDYIFG cleared
			masterRxData[masterRxCurrentCnt] = UCB0RXBUF;
			// when last byte is BEING received generate STOP Condition
			// I think below will not work when byteCnt == 2 -> Working
			if ((masterRxCurrentCnt) == (lastCount-1))
			{
				UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition right after sending last data
			}

		}
	}
	// if byteCount == 1
	else if (byteCount == 1)
	{
		UCB0CTL1 |= UCTXSTT;                      // I2C start condition
		while(UCB0CTL1 & UCTXSTT)
		{
			// check Arbitration Lost before NACK as when Arbitration is lost you cannot send STOP after NACK
			if(i2c_checkArbLost())
			{
				i2cMasterStatus = I2C_ARB_LOST;
				return 0;
			}

			// check NACK before checking timeout to prevent timeout when we have NACK
			if(i2c_checkNACK())
			{
				i2cMasterStatus = I2C_NACK;
				return 0;
			}

			if(i2c_checkTimeout())
			{
				i2cMasterStatus = I2C_TIMEOUT;;
				return 0;
			}
		}
		UCB0CTL1 |= UCTXSTP;                      // I2C stop condition
		masterRxData[0] = UCB0RXBUF;
	}

	i2cMasterStatus = i2c_waitForPreviousTransfer(byteCount);
	if (i2cMasterStatus != I2C_TRANSFER_DONE)
	{
		return 0;
	}


	// After STOP condition is transfered, disable all i2c interrupt
	i2c_disableAllInterrupt();

	// timer stop after sending STOP condition
	i2c_timerTimeoutStop();

	// when everything goes well we return final count
	// value 0 means something wrong
	return masterTxCurrentCnt;
}


void i2c_slaveInit(uint8_t ownAddress, uint8_t dataSize, uint8_t *data)
{
	// disable all i2c interrupt in the beginning
	i2c_disableAllInterrupt();

	// initialize buffer and count
	i2cInteralAddress = 0; 						// Init internal address
	slaveIndex = 0;								// reset slave index
	slaveBufSize = dataSize;					// Store slave buffer size
	slaveData = data;							// store buffer address
	defaultSlaveData = data;					// Store default slave data address

	// TODO: check whether slave also need to configure the clock!!
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0I2COA = ownAddress;					// store own address
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
}

void i2c_slaveStart(void)
{
	// init I2C slave status
	i2cSlaveTxDone = 0;
	i2cSlaveRxDone = 0;
	i2cSlaveTxInProgress = 0;
	i2cSlaveRxInProgress = 0;
	i2cStartDetected = 0;
	i2cGeneralCallDetected = 0;

	IE2 |= UCB0TXIE + UCB0RXIE;               // Enable TX interrupt
	UCB0I2CIE |= UCSTTIE+UCSTPIE;                     // Enable START and STOP interrupt
	//enalbe general call interrupt
	UCB0I2COA |= UCGCEN;
}

uint8_t i2c_getInternalAddress(void)
{
	return i2cInteralAddress;
}

uint8_t i2c_getSlaveTxDone(void)
{
	return i2cSlaveTxDone;
}

void i2c_setSlaveTxDone(uint8_t status)
{
	i2cSlaveTxDone = status;
}

uint8_t i2c_getSlaveRxDone(void)
{
	return i2cSlaveRxDone;
}

void i2c_setSlaveRxDone(uint8_t status)
{
	i2cSlaveRxDone = status;
}

uint8_t i2c_getGeneralCallDetected(void)
{
	return i2cGeneralCallDetected;
}

void i2c_setGeneralCallDetected(uint8_t status)
{
	   i2cGeneralCallDetected = status;
}

uint8_t i2c_checkInternalAddress(uint8_t addr)
{
	if (addr >= 0 && addr <8)
		return 1;
	else
		return 0;
}

void i2c_busRecovery(void)
{
	volatile uint8_t i;
	P3SEL &= ~(I2C_SDA_PIN + I2C_SCL_PIN);		// set pins to GPIO
	P3DIR |= I2C_SDA_PIN + I2C_SCL_PIN;			// set output direction
	P3OUT |= I2C_SDA_PIN;						// set SDA HIGH
	for (i = 0 ; i < 9 ; i++)
	{
		P3OUT |= I2C_SCL_PIN;					// toggle SCL 9 times
		// TODO: delay us
		P3OUT &= ~I2C_SCL_PIN;
		// TODO: delay us
	}
	P3SEL |= I2C_SDA_PIN + I2C_SCL_PIN;			// set pins back to I2C
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	// uart  RX interrupt
	if ((IFG2 & UCA0RXIFG) && (IE2 & UCA0RXIE))
	{
		uart_rxByte = UCA0RXBUF;					// Get the received character
		// gps (uart)
		uart_setRxFlag();
		__bic_SR_register_on_exit(LPM3_bits);
	}

	// Master Mode: when NACK is detected
	if (UCB0STAT & UCNACKIFG)
	{   // send STOP if slave sends NACK
		if (UCB0CTL0 & UCMST)
		{
			UCB0CTL1 |= UCTXSTP;
		}
	}

	if(UCB0STAT & UCSTPIFG)
	{
		UCB0STAT &= ~UCSTPIFG;
	   // if receive mode
	   if(i2cSlaveRxInProgress)
	   {
		   i2cSlaveRxInProgress = 0;
		   i2cSlaveRxDone = 1;
		   // TODO: fix exit LPM policy
		   __bic_SR_register_on_exit(LPM3_bits);
	   }

	   // if transmit mode
	   if(i2cSlaveTxInProgress)
	   {
		   i2cSlaveTxInProgress = 0;
		   i2cSlaveTxDone = 1;
	   }
	   __bic_SR_register_on_exit(LPM3_bits);
	}

	if (UCB0STAT & UCGC)
	{
		UCB0STAT &= ~UCGC;
		i2cGeneralCallDetected = 1;
		__bic_SR_register_on_exit(LPM3_bits);
	}

	// Slave mode: when start condition is detected
	if (UCB0STAT & UCSTTIFG)
	{
		UCB0STAT &= ~UCSTTIFG;                    // Clear start condition int flag
		i2cStartDetected = 1;
		// initialize something
		 slaveIndex = 0;							// Initialize the index counter for slave mode
		 slaveData = defaultSlaveData;			// Initialize the base address of slave buffer
	}

}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	P2OUT |= EXTWDT_PIN;

	// uart  TX interrupt
	if ((IFG2 & UCA0TXIFG) && (IE2 & UCA0TXIE))
	{
		IE2 &= ~UCA0TXIE;
	}

	// i2c RX interrupt
	if (IFG2 & UCB0RXIFG)
	{
		// Master
		if (UCB0CTL0 & UCMST)
		{

		}
		else
		{
		   if (i2cStartDetected)
		   {
			   i2cStartDetected = 0;
			   i2cSlaveRxInProgress = 1;
			   i2cInteralAddress = UCB0RXBUF;

			   // boundary check
			   if((i2cInteralAddress >=0) && (i2cInteralAddress < slaveBufSize))
			   {
				   slaveData = slaveData + i2cInteralAddress;
				   slaveIndex = i2cInteralAddress;
			   }
			   else
			   {
				   // if internal address is not matched, set to default address
				   // TODO when internal address is not matched
				   // this is possibly Command from I2C Master
				   i2cInteralAddress = 0x00;
				   slaveIndex = 0;
				   // from the datasheet, USART module cannot send NACK when in Slave mode
				   // so make master sure to send right internal address otherwise master will read trash value
				   // TODO: do something when slave has wrong internal address
			   }
		   }
		   else
		   {
			   // check Overflows
			   if(slaveIndex < slaveBufSize)
			   {
				   *slaveData = UCB0RXBUF;
				   slaveData++;
				   slaveIndex++;
			   }
			   else
			   {
				   volatile uint8_t dummy;
				   // To clear the interrupt flag, write data to dummy
				   dummy = UCB0RXBUF;
			   }
		   }
		}
	}

	// i2c TX interrupt
	if (IFG2 & UCB0TXIFG)
	{
		// Master
		if (UCB0CTL0 & UCMST)
		{

		}
		else
		{
		   if(i2cStartDetected)
		   {
			   i2cStartDetected = 0;
			   i2cSlaveTxInProgress = 1;
			   slaveData = slaveData + i2cInteralAddress;
			   slaveIndex = i2cInteralAddress;
			   UCB0TXBUF = *slaveData;
			   slaveData++;
			   slaveIndex++;
		   }
		   else
		   {
			   // check overflows
			   if(slaveIndex < slaveBufSize)
			   {
				   UCB0TXBUF = *slaveData;
				   slaveData++;
				   slaveIndex++;
			   }
			   else
			   {
				   UCB0TXBUF = 0xFF;
			   }
		   }

		}
	}
	P2OUT &= ~EXTWDT_PIN;
}
