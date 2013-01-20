/*
 * main.c
 */
#include "ossibeacon.h"
#include "printf.h"
#include "adf7012.h"
#include "aclkuart.h"
#include "i2c.h"
#include "ossi_morse.h"
#include "ossi_gps.h"
#include "adc10.h"

void beacon_data_receive(void);
void beacon_data_processing(void);
void beacon_data_send(void);

void main(void)
{
	//first thing to do
	int_wdt_disable();
	clock_setup();

	//ports setup
	IO_setup();
	ext_wdt_setup();
	uart_setup_9600();
	//adc10_setup(ADC10_PIN_2_0 + ADC10_PIN_2_1);
	adf7012_setup();

	//module init
	uart_init();
	// TODO: check WDI timing
	ext_wdt_rst();


	// wait 1000 ms in the beginning for stabilizing 32.768kHz
	// TODO:implement ACLK clock stability check ->
	delay_ms(1);
//	morse_init(20);
//	ADF7012_OOK(HIGH);
//	printf("system on\r\n");
	while(1)
	{
		//printf("VBUS: %u\r\n",adc10_read());
		// Enter LPM3, interrupts enabled
		__bis_SR_register(LPM3_bits + GIE);
		//adc10_enable_int();
		//adc10_start(INCH_0);
		beacon_data_receive();
		beacon_data_processing();
		beacon_data_send();
	}
}


void beacon_data_receive(void) // uart related handler
{
	// gps
	if (uart_rx_ready())
	{
		uart_clear_rxFlag();
		// uart_get_byte() only works after uart ISR
		// TODO:check possibility of error!!!
		if(gps_updateData(uart_get_byte()))
		{
			// if all the gps data we want are received
			gps_setReadyFlag();
			return;
		}
	}
	else
	{
		return;
	}

	// i2c
	if (i2c_rx_ready())
	{
		i2c_clear_rxFlag();
	}
}

void beacon_data_processing(void)
{
	//gps
	// gps
	if (gps_isReady())
	{
		gps_clearReadyFlag();
		// process gps data
		// make packet from i2c and gps data

		gps_makePacket();
		// ready to send morse code
		morse_init();
	}
	else
	{
		return;
	}


	// i2c
	if (i2c_is_ready())
	{
		i2c_clear_readyFlag();
		morse_init();

	}
}

void beacon_data_send(void) // timer0 related handler
{
	if(morse_isReady())
	{
		// clear the flag
		morse_clearSendFlag();
		morse_sendBytes(gps_getStream());
	}
	else
	{
		return;
	}

}
