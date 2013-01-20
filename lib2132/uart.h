/*
 * aclkuart.h
 *
 *  Created on: 2012. 12. 19.
 *      Author: OSSI
 */

#ifndef ACLKUART_H_
#define ACLKUART_H_

#include "ossi_beacon.h"


void uart_setupACLK4800(void);
void uart_setupACLK9600(void);
void uart_init(void);

void uart_setRxFlag(void);
void uart_clearRxFlag(void);
uint8_t uart_rxReady(void);

void i2c_set_rxFlag(void);
void i2c_clear_rxFlag(void);
uint8_t i2c_rx_ready(void);

uint8_t uart_getByte(void);

void putc(unsigned);
void puts(char *);




#endif /* ACLKUART_H_ */
