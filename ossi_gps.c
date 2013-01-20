/*
 * gps.c
 *
 *  Created on: 2012. 12. 21.
 *      Author: OSSI
 *      Inspired by: http://rev0proto.com/wiki/index.php/Senior_Project / Justin Kenny
 */

#include "ossi_gps.h"

static volatile uint8_t gpsReadyFlag = 0;

uint8_t rawgps[82];
uint8_t gpsgga[82];
uint8_t gpsIndex = 0;
uint8_t gpsDone = 0;
Gps gpsData = {"3518.5230", "12039.6090", "123456.789", "00", "000000.0", 'N', 'W', '0'};

uint8_t gpsArrayData[64]={0}; // 59 = sizeof(gpsdata) + spacer character

void gps_setReadyFlag(void)
{
	gpsReadyFlag = 1;
}

void gps_clearReadyFlag(void)
{
	gpsReadyFlag = 0;
}

uint8_t gps_isReady(void)
{
	if(gpsReadyFlag)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

void gps_parseNMEA(void)
{
	IE2 &= ~UCA0RXIE; //Disable receive interrupt
	uint8_t commas[MAXCOMMAS];
	uint8_t i = 0, n;
	for(n=0;n<MAXCOMMAS;n++) //Find the positions of all commas in the NMEA sentence, put positions in commas[]
	{
	   for(;rawgps[i]!=','||i>MAXGPS;i++); //Find next comma; continue stepping through  the array until we find 0x2C (,)
	   commas[n] = i; //Store the index in commas[] array
	   i++;
	}
	if(rawgps[commas[5]+1] != '0') //Make sure we have GPS fix; 0 = invalid
	{
	   for(i=commas[1]+1;i<commas[2];i++)
	   {
			   gpsData.lat[i-(commas[1]+1)] = rawgps[i]; //Load latitude into lat[] array from stored NMEA string
	   }
	   //gpsdata.lat[i-(commas[1])-1] = '\0';
	   gpsData.ns = rawgps[commas[2]+1];

	   for(i=commas[3]+1;i<commas[4];i++)
	   {
			   gpsData.lng[i-(commas[3]+1)] = rawgps[i]; //Load longitude into lng[] array from stored NMEA string
	   }
	   //gpsdata.lng[i-(commas[3])-1] = '\0';
	   gpsData.ew = rawgps[commas[4]+1];

	   for(i=commas[6]+1;i<commas[7];i++)
	   {
			   gpsData.sats[i-(commas[6]+1)] = rawgps[i]; //Load sats into sats[] array from stored NMEA string
	   }
	   //gpsdata.sats[i-(commas[6])-1] = '\0';

	   for(i=commas[8]+1;i<commas[9];i++)
	   {
			   gpsData.alti[i-(commas[8]+1)] = rawgps[i]; //Load alt into alti[] array from stored NMEA string
	   }
	   //gpsdata.alti[i-(commas[8])-1] = '\0';

	   for(i=commas[0]+1;i<commas[1];i++)
	   {
			   gpsData.time[i-(commas[0]+1)] = rawgps[i]; //Load time into time[] array from stored NMEA string
	   }
	   //gpsdata.time[i-(commas[0])-1] = '\0';
	   gpsData.valid = rawgps[commas[5]+1];
	}
	else //Else update the timestamp, but retain old GPS data
	{
	   for(i=commas[0]+1;i<commas[1];i++)
	   {
			   gpsData.time[i-(commas[0]+1)] = rawgps[i];
	   }
	   //gpsdata.time[i-(commas[0])-1] = '\0';

	   for(i=commas[6]+1;i<commas[7];i++)
	   {
			   gpsData.sats[i-(commas[6]+1)] = rawgps[i]; //Load sats into sats[] array from stored NMEA string
	   }
	   //gpsdata.sats[i-(commas[6])-1] = '\0';
	   gpsData.valid = '0';
	}
	IE2 |= UCA0RXIE; //Enable receive interrupt
}

void gps_makePacket(void)
{
	volatile uint8_t i;
	volatile uint8_t step = 0;


	gpsArrayData[step] ='C';
	step++;
	gpsArrayData[step] ='Q';
	step++;
	gpsArrayData[step] ='C';
	step++;
	gpsArrayData[step] ='Q';
	step++;
	gpsArrayData[step] ='C';
	step++;
	gpsArrayData[step] ='Q';
	step++;

	gpsArrayData[step] =' ';
	step++;

	gpsArrayData[step] ='D';
	step++;

	gpsArrayData[step] ='E';
	step++;

	gpsArrayData[step] =' ';
	step++;

	gpsArrayData[step] ='O';
	step++;

	gpsArrayData[step] ='S';
	step++;

	gpsArrayData[step] ='S';
	step++;

	gpsArrayData[step] ='I';
	step++;

	gpsArrayData[step] =' ';
	step++;


	for (i = 0; i < 9 ; i++) {
		gpsArrayData[step] = gpsData.lat[i];
		step++;
	}

	gpsArrayData[step] =' ';
	step++;

	for (i = 0; i < 10 ; i++) {
		gpsArrayData[step] = gpsData.lng[i];
		step++;
	}

	gpsArrayData[step] =' ';
	step++;

//	for (i = 0; i < 10 ; i++) {
//		gps_array_data[step] = gpsdata.time[i];
//		step++;
//	}
//
//	gps_array_data[step] =',';
//	step++;

	for (i = 0; i < 2 ; i++) {
		gpsArrayData[step] = gpsData.sats[i];
		step++;
	}

	gpsArrayData[step] =' ';
	step++;

	for (i = 0; i < 8 ; i++) {
		gpsArrayData[step] = gpsData.alti[i];
		step++;
	}

	gpsArrayData[step] =' ';
	step++;

	gpsArrayData[step] = gpsData.ns;
	step++;
	gpsArrayData[step] = gpsData.ew;
	step++;
	gpsArrayData[step] = gpsData.valid;

}


uint8_t gps_updateData(uint8_t data)
{
	if(data == '$') //$ = Start of NMEA Sentence
	{
		gpsIndex = 0;
		gpsDone = 0;
	}
	else if(data == '*') //<CR> = End of Transmission
	{
		gpsDone = 1;
		if(rawgps[4] == 'G') //Make sure this is a GGA sentence
		{
			gps_parseNMEA();
			return 1;
		}
	}
	if(gpsDone != 1)
	{
		rawgps[gpsIndex] = data;
		gpsIndex++;
	}
	if(gpsIndex > 81)
	{
		gpsIndex = 0;
	}
	return 0;
}

uint8_t* gps_getStream(void)
{
	return gpsArrayData;
}
