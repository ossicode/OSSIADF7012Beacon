/*
 * morse.h
 *
 *  Created on: 2012. 12. 19.
 *      Author: OSSI
 */

#ifndef MORSE_H_
#define MORSE_H_

#include "ossi_beacon.h"
#include "adf7012.h"

//#define MAX_DATA_SIZE 59

void morse_setSendFlag(void);
void morse_clearSendFlag(void);

//void morse_set_WPM(void);

void morse_init(void);
uint8_t morse_isReady(void);
void morse_sendBytes(uint8_t * bytes);

#endif /* MORSE_H_ */
