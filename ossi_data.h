/*
 * ossi_data.h
 *
 *  Created on: 2013. 1. 23.
 *      Author: OSSI
 */

#include "ossi_types.h"

#ifndef OSSI_DATA_H_
#define OSSI_DATA_H_

/*
 * OBC Data Model
 */

#define OSSI_DATA_SIZE		(64)
#define OBC_DATA_SIZE		(48)
#define COMMS_DATA_SIZE		(16)
#define BEACON_DATA_SIZE	(16)
#define LED_DATA_SIZE		(16)


#define OBC_DATA_START_ADDR	(0)
#define COMMS_DATA_START_ADDR	(OSSI_DATA_SIZE - COMMS_DATA_SIZE)
#define BEACON_DATA_START_ADDR	(OSSI_DATA_SIZE - BEACON_DATA_SIZE)
#define LED_DATA_START_ADDR		(OSSI_DATA_SIZE - LED_DATA_SIZE)



#define STATUS_DATA_ADDR	(0)
#define STATUS_DATA_SIZE	(8)

#define VI_DATA_ADDR	(8)
#define VI_DATA_SIZE	(10)

#define TEMP_DATA_ADDR	(18)
#define TEMP_DATA_SIZE	(10)

#define MODULE_DATA_ADDR	(28)
#define MODULE_DATA_SIZE	(10)

#define LEDTIME_DATA_ADDR	(38)
#define LEDTIME_DATA_SIZE	(4)

enum parmOperationMode
{
	BOOT_MODE = 0,
	DEPLOY_MODE = 1,
	NORMAL_MODE = 2,
	LED_MODE = 3,
	SAFE_MODE = 4,
	RECOVERY_MODE= 5,
	TEST_MODE = 6
};

enum parmEpsStatus
{
	SOLAR_MODE = 0,
	BATTERY_MODE = 1,
	SUPERCAP_MODE = 2,
	BATTERY_CHARGED = 0x08,
	BATTERY_FAULT = 0x10,
	SUPERCAP_CHARGER_EN = 0x20,
	SUPERCAP_CHARGED = 0x40,
	SUPERCAP_FAULT = 0x80
};

//enum parmErrStatus
//{
//
//};

typedef union {
	uint8_t data[STATUS_DATA_SIZE];
	struct {
		uint8_t operationMode;
		uint8_t onStatus;
		uint8_t epsStatus;
		uint8_t errStatus;
		uint8_t obcTick3;
		uint8_t obcTick2;
		uint8_t obcTick1;
		uint8_t obcTick0;
	} u8;
} generaStatus_t;

typedef union {
	uint8_t data[VI_DATA_SIZE];
	struct {
		uint8_t vsolar;
		uint8_t vbat;
		uint8_t vbus;
		uint8_t vsupercap;
		uint8_t solarCs5PlusY;
		uint8_t solarCs4PlusZ;
		uint8_t solarCs3MinusX;
		uint8_t solarCs2MinusY;
		uint8_t solarCs1MinusZ;
		uint8_t busCs;
	} u8;
} viData_t;

typedef union {
	uint8_t data[TEMP_DATA_SIZE];
	struct {
		uint8_t battery;
		uint8_t supercap;
		uint8_t comms;
		uint8_t beacon;
		uint8_t obc;
		uint8_t plusY;
		uint8_t plusZ;
		uint8_t minusX;
		uint8_t minusY;
		uint8_t minusZ;
	} u8;
} tempData_t;

typedef union {
	uint8_t data[MODULE_DATA_SIZE];
	struct {
		uint8_t commRxCmd;
		uint8_t commsRxCnt;
		uint8_t commsRssi;
		uint8_t commsTxDataId;
		uint8_t gyroSensor;
		uint8_t sunSensor;
		uint8_t obcLatchUpCnt;
		uint8_t commsLatchUpCnt;
		uint8_t beaconLatchUpCnt;
		uint8_t ledLatchUpCnt;
	} u8;
} moduleData_t;

typedef union {
	uint8_t data[LEDTIME_DATA_SIZE];
	struct {
		uint8_t date;
		uint8_t hour;
		uint8_t min;
		uint8_t ledSensor;
	} u8;
} ledTimeData_t;

/*
 * Shared Status
 */

enum parmTxStatus
{
	TX_STAND_BY= 0,
	SENDING = 128,
	SENT = 156
};

enum parmPllLock
{
	PLL_NOT_LOCK = 0,
	PLL_LOCKED = 128
};

enum parmPaFault
{
	PA_NOT_FAULT = 0,
	PA_FAULT = 128
};


/*
 * COMMS Data Model
 */
#define COMMS_RX_CMD_ADDR		(COMMS_DATA_START_ADDR)
#define COMMS_RX_CNT_ADDR		(COMMS_DATA_START_ADDR+1)
#define COMMS_RSSI_ADDR			(COMMS_DATA_START_ADDR+2)
#define COMMS_TX_DATA_ID_ADDR	(COMMS_DATA_START_ADDR+3)
#define OBC_LATCH_UP_CNT_ADDR	(COMMS_DATA_START_ADDR+4)
#define COMMS_RX_STATUS_ADDR	(COMMS_DATA_START_ADDR+5)
#define COMMS_TX_STATUS_ADDR	(COMMS_DATA_START_ADDR+6)
#define COMMS_TEMP_ADDR			(COMMS_DATA_START_ADDR+7)
#define COMMS_PLL_LOCK_ADDR		(COMMS_DATA_START_ADDR+8)
#define COMMS_PA_FAULT_ADDR		(COMMS_DATA_START_ADDR+9)
// reserved
#define COMMS_CMD1_ADDR			(COMMS_DATA_START_ADDR+13)
#define COMMS_CMD2_ADDR			(COMMS_DATA_START_ADDR+14)
#define COMMS_INT_ADDR_ADDR		(COMMS_DATA_START_ADDR+15)

//enum parmCommsRxCmd
//{
//
//};

//enum parmCommsTxDataId
//{
//
//};

enum parmCommsRxStatus
{
	RX_STAND_BY= 0,
	RECEIVING = 128,
	RECEIVED = 156
};

enum parmCommsCmd1
{
	COMMS_CMD1_CLEAR = 0,
	TX_SEND_START = 128,
	TX_SEND_STOP =129,
	DRAIN_BATTERY = 130 // when the system needs full Power Reboot
};

typedef union {
	uint8_t data[COMMS_DATA_SIZE];
	struct {
		uint8_t commRxCmd;
		uint8_t commsRxCnt;
		uint8_t commsRssi;
		uint8_t commsTxDataId;
		uint8_t obcLatchUpCnt;
		uint8_t commsRxStatus;
		uint8_t commsTxStatus;
		uint8_t commsTemp;
		uint8_t pllLock;
		uint8_t paFault;
		uint8_t reserved[3];
		uint8_t commsCmd1;
		uint8_t commsCmd2;
		uint8_t internalAddr;
	} u8;
} commsModule_t;

/*
 * Beacon Data Model
 */
#define BEACON_TX_STATUS_ADDR	(BEACON_DATA_START_ADDR)
#define BEACON_VBUS_ADDR		(BEACON_DATA_START_ADDR+1)
#define BEACON_ADC1_ADDR		(BEACON_DATA_START_ADDR+2)
#define BEACON_TEMP_ADDR		(BEACON_DATA_START_ADDR+3)
#define BEACON_PLL_LOCK_ADDR	(BEACON_DATA_START_ADDR+4)
#define BEACON_PA_FAULT			(BEACON_DATA_START_ADDR+5)
// reserved
#define BEACON_CMD1_ADDR		(BEACON_DATA_START_ADDR+13)
#define BEACON_CMD2_ADDR		(BEACON_DATA_START_ADDR+14)
#define BEACON_INT_ADDR_ADDR	(BEACON_DATA_START_ADDR+15)


enum parmBeaconCmd1
{
	BEACON_CMD1_CLEAR = 0,
	MORSE_SEND_START = 128,
	MORSE_SEND_STOP =129
};

typedef union {
	uint8_t data[BEACON_DATA_SIZE];
	struct {
		uint8_t beaconTxStatus;
		uint8_t vbus;
		uint8_t adc1;
		uint8_t beaconTemp;
		uint8_t pllLock;
		uint8_t paFault;
		uint8_t reserved[7];
		uint8_t beaconCmd1;
		uint8_t beaconCmd2;
		uint8_t internalAddr;
	} u8;
} beaconModule_t;


/*
 * LED Data Model
 */
#define LED_TX_STATUS_ADDR		(LED_DATA_START_ADDR)
#define LED_VSUPERCAP_ADDR		(LED_DATA_START_ADDR+1)
#define LED_SUPERCAP_CS_ADDR	(LED_DATA_START_ADDR+2)
#define LED_TEMP_ADDR			(LED_DATA_START_ADDR+3)
// reserved
#define LED_CMD1_ADDR			(LED_DATA_START_ADDR+13)
#define LED_CMD2_ADDR			(LED_DATA_START_ADDR+14)
#define LED_INT_ADDR_ADDR		(LED_DATA_START_ADDR+15)

enum parmLedCmd1
{
	LED_CMD1_CLEAR = 0,
	LED_MSG_START = 128,
	LED_MSG_STOP =129
};

typedef union {
	uint8_t data[LED_DATA_SIZE];
	struct {
		uint8_t ledTxStatus;
		uint8_t vsupercap;
		uint8_t supercapCs;
		uint8_t ledTemp;
		uint8_t reserved[9];
		uint8_t ledCmd1;
		uint8_t ledCmd2;
		uint8_t internalAddr;
	} u8;
} ledModule_t;

#endif /* OSSI_DATA_H_ */
