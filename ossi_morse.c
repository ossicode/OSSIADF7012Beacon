/*
 * morse.c
 *
 *  Created on: 2012. 12. 19.
 *      Author: OSSI
 */

#include "ossi_morse.h"

//                      space .    ,    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O    P    Q    R    S    T    U    V    W    X    Y    Z
const uint8_t code[39] ={0x00,0x15,0x33,0x1F,0x0F,0x07,0x03,0x01,0x00,0x10,0x18,0x1C,0x1E,0x01,0x08,0x0A,0x04,0x00,0x02,0x06,0x00,0x00,0x07,0x05,0x04,0x03,0x02,0x07,0x06,0x0D,0x02,0x00,0x01,0x01,0x01,0x03,0x09,0x0B,0x0C};
const uint8_t size[39] ={4,6,6,5,5,5,5,5,5,5,5,5,5,2,4,4,3,1,4,3,4,2,4,3,4,2,2,3,4,4,3,3,1,3,4,3,4,4,4};

static volatile uint8_t morseSendFlag = 0;

static volatile uint16_t dotLength;
static volatile uint16_t totalDotLength;
static volatile uint16_t tick;

static volatile uint16_t bytesSize = 0;

//void morse_timerStart(void)
//{
//	// sending morse code
//
//	// default: set to 12 WPM, 1 dot = 100 ms //
//	// 1 WPM = 50 dots per min
//	// ex) 1 WPM, 1 dot = 1200ms
//	tick = 0;
//	TA0CCR0 = 3276;						       // 3276 = 100 ms
////	TA0CCR0 = 1966; // 60 ms
//	TA0CTL = TASSEL_1 + MC_1;                  // ACLK, upmode
//	TA0CCTL0 |= CCIE;                          // TA0CCR0 interrupt enabled
//}

//void morse_set_WPM(uint8_t wpm)
//{
//	//min WPM = 1, 1 dot = 1200 ms
//	//max WPM = 50, 1 dot = 24 ms
//	//OSSI WPM = 12, 1 dot = 100 ms
//	min_max(1,50,wpm);
//	dot_length = (uint16_t)(1200 / wpm);
//}

void morse_setSendFlag(void)
{
	morseSendFlag = 1;
}

void morse_clearSendFlag(void)
{
	morseSendFlag = 0;
}

uint8_t morse_isReady(void)
{
	if(morseSendFlag)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint16_t morse_setDataSizeFrom(uint8_t * bytes)
{
	volatile uint16_t bytes_size = 0;
	while(*bytes++)
	{
		bytes_size++;
	}

	return bytes_size;
}

void morse_init(void)
{
	// TODO: add extra initialization process if needed
//	morse_set_WPM(wpm);
	adf7012_init();
	adf7012_initAllRegisters();
	adf7012_enable();
	adf7012_writeAllRegisters();
	if(adf7012_lock())
	{
		adf7012_setPALevel(63);
		adf7012_setPAON(1);
		adf7012_enable();
		adf7012_writeAllRegisters();
		morse_setSendFlag();
		return;
	}
	else
	{
		// TODO: do something when PLL is not locked!!
		// set vco bias and adj value with best guess for PLL
		morse_clearSendFlag();
		adf7012_disable();

	}
}

void morse_initLowPower(void)
{
	// TODO: add extra initialization process if needed
//	morse_set_WPM(wpm);
	adf7012_init();
	adf7012_initAllRegisters();
	adf7012_enable();
	adf7012_writeAllRegisters();
	if(adf7012_lock())
	{
		morse_setSendFlag();
		adf7012_disable();
		return;
	}
	else
	{
		// TODO: do something when PLL is not locked!!
		// set vco bias and adj value with best guess for PLL
		morse_clearSendFlag();
		adf7012_disable();

	}
}

void morse_sendDots(uint8_t dots, uint8_t val)
{
//	totalDotLength = 0;
//	totalDotLength = dots;
	if (val)
	{
		P1OUT |= BEACON_DATA_PIN;
		P3OUT |= LED_PIN;
	}
	else
	{
		P1OUT &= ~BEACON_DATA_PIN;
		P3OUT &= ~LED_PIN;
	}
	systimer_msDelay(dots * 100); // 100 ms = 1 dot -> 12 WPM
//	morse_timerStart();
}

//void morse_sendDotsLowpower(uint8_t dots, uint8_t val)
//{
////	totalDotLength = 0;
////	totalDotLength = dots;
//	if (val)
//	{
//		adf7012_OOK(val);
//		P3OUT |= LED_PIN;
//	}
//	else
//	{
//		adf7012_OOK(val);
//		P3OUT &= ~LED_PIN;
//	}
//	systimer_msDelay(dots*100);
////	morse_timerStart();
//}

void morse_sendByte(uint8_t byte)
{
	volatile uint8_t convIndex;
	volatile uint8_t dotCnt;
	volatile uint8_t totalDots;
	volatile uint8_t converted;

	//if numbers
	if(byte >= '0' && byte <= '9')
	{
		convIndex = 45;
	}
	else
	// space
	if(byte == ' ')
	{
//		convIndex = 32;
		// previous 3 dots + 2 dots + 2 dots  = 7 dot pause
		morse_sendDots(2,0);
		// when space, return
		return;
	}
	else
	//if Capital Alphabet letter
	if(byte >= 'A' && byte <= 'Z')
	{
		convIndex = 52;
	}
	else
	//if Capital Alphabet letter
	if(byte == ',' )
	{
		convIndex = 42;
	}
	else
	//if Capital Alphabet letter
	if(byte == '.' )
	{
		convIndex = 45;
	}

	totalDots = size[byte-convIndex];
	converted = code[byte-convIndex];

	for (dotCnt = 0; dotCnt < totalDots ; dotCnt++)
	{
		if ((converted >> (totalDots - dotCnt - 1)) & 0x01)
		{
			// 3 dots
			morse_sendDots(3,1);
		}
		else
		{
			// 1 dot
			morse_sendDots(1,1);
		}
		// 1 dot pause
		morse_sendDots(1,0);
	}
}

void morse_send(uint8_t* bytes)
{
	volatile uint8_t bytesCnt;
	volatile uint8_t dataSize;

	dataSize = morse_setDataSizeFrom(bytes);
	for(bytesCnt = 0; bytesCnt < dataSize ; bytesCnt++ )
	{
		morse_sendByte(bytes[bytesCnt]);
		morse_sendDots(2,0); // send 2 more dots so became total of 3 dots
	}

	adf7012_disable();
//	return;
}

//void morse_sendBytes(uint8_t * bytes)
//{
//	// TODO: check initialization of static variables
//	static volatile uint16_t bytes_cnt =0;
//	static volatile uint16_t dotCnt =0;
//	static volatile uint8_t dot_sent = 0;
//	volatile uint16_t MAX_DATA_SIZE;
//
//	MAX_DATA_SIZE = morse_setDataSizeFrom(bytes);
//
//	// size[bytes[bytes_cnt]-65]: number of dots to send including pause dots
//	if( bytes_cnt < MAX_DATA_SIZE)
//	{
//
//		volatile uint8_t convIndex;
//
//		//if Capital Alphabet letter
//		if(bytes[bytes_cnt] >= 'A' && bytes[bytes_cnt] <= 'Z')
//		{
//			convIndex = 52;
//		}
//
//		//if numbers
//		if(bytes[bytes_cnt] >= '0' && bytes[bytes_cnt] <= '9')
//		{
//			convIndex = 45;
//		}
//
//		//if Capital Alphabet letter
//		if(bytes[bytes_cnt] == ',' )
//		{
//			convIndex = 42;
//		}
//
//		//if Capital Alphabet letter
//		if(bytes[bytes_cnt] == '.' )
//		{
//			convIndex = 45;
//		}
//
//		if(bytes[bytes_cnt] == ' ' )
//		{
//			dotCnt = 0;
//			bytes_cnt++;
//			morse_sendDots(4,0);
//			return;
//		}
//
//		volatile uint8_t totalDots = size[bytes[bytes_cnt]-convIndex];
//		volatile uint8_t converted = code[bytes[bytes_cnt]-convIndex];
//
//		if( dotCnt < totalDots ) // count dots with pause added
//		{
//			if(dot_sent == 0)
//			{
//				dot_sent = 1;
//				//check dash(1) or dot(0) from MSB
//				if ((converted >> (totalDots - dotCnt - 1)) & 0x01)
//				{
//					// 3 dots
//					morse_sendDots(3,1);
//					return;
//				}
//				else
//				{
//					// 1 dot
//					morse_sendDots(1,1);
//					return;
//				}
//			}
//
//			if(dot_sent == 1)
//			{
//				dot_sent = 0;
//				dotCnt++;
//				// if dots are sent
//				// send 1 dot pause
//				morse_sendDots(1,0);
//				return;
//			}
//		}
//		else // if a byte is sent
//		{
//				dotCnt = 0;
//				bytes_cnt++;
//				morse_sendDots(2,0); // send 2 more dots so became total of 3 dots
//				return;
//		}
//	}
//	else // if all bytes are sent
//	{
//		dotCnt = 0;
//		bytes_cnt = 0;
//		adf7012_disable();
//		return;
//	}
//
//}

// Timer0_A0 interrupt service routine
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void Timer_A (void)
//{
//	// data processing
//	// increase tick
//	// compare tick to dotCnt
//	// data processing
//	tick++; // every 100 ms
//
//	if (tick > totalDotLength-1)
//	{
//		tick = 0;
//		// morse_set_sendFlag();
//		morseSendFlag = 1; // replacing function call in ISR
//		TA0CCTL0 &= ~CCIE; // disable timer interrupt
//		// exit LPM3
//		__bic_SR_register_on_exit(LPM3_bits);
//	}
//}
